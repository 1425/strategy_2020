#include<functional>
#include<cmath>
#include "util.h"
#include "int_limited.h"
#include "dist.h"
#include "input_data.h"

/*To do list:
 * 1) different strategies about who is climbing effects time to do other things
 * 2) Improve quality of random robot match data
 * 3) Make the climb being balanced do something
 * 4) Check for consistency of the thing being balanced on input data
 * */

//start generic code

using namespace std;

template<typename T>
std::vector<std::pair<T,bool>> mark_end(std::vector<T> a){
	return mapf(
		[=](auto p){
			auto [i,v]=p;
			return make_pair(v,i==a.size()-1);
		},
		enumerate(a)
	);
}

std::string join(std::string delim,std::vector<std::string> const& a){
	std::stringstream ss;
	for(auto [elem,last]:mark_end(a)){
		ss<<elem;
		if(!last){
			ss<<delim;
		}
	}
	return ss.str();
}

double product(std::vector<double> const& a){
	double r=1;
	for(auto elem:a) r*=elem;
	return r;
}

double geomean(std::vector<double> const& a){
	assert(a.size());
	return pow(product(a),1.0/a.size());
}

//start program-specific code

#define CLIMB_CAPABILITIES(X)\
	X(Px,climb_unassisted)\
	X(Px,assist2)\
	X(Px,assist1)\
	X(Px,climb_was_assisted)\
	X(Px,park)\
	X(Px,balance)

#define ROBOT_CAPABILITIES(X)\
	X(Px,auto_line)\
	X(double,auto_low)\
	X(double,auto_outer)\
	X(double,auto_inner)\
	X(double,tele_low)\
	X(double,tele_outer)\
	X(double,tele_inner)\
	X(Dist,auto_ball_dist)\
	X(Dist,tele_ball_dist)\
	X(Px,wheel_spin)\
	X(Px,wheel_color)\
	CLIMB_CAPABILITIES(X)

struct Robot_capabilities{
	ROBOT_CAPABILITIES(INST)
};

/*
climb strategy:
1) all on own
2) each one attempting to assist each of the others
3) just park
map<Team,park/climb/assisted_climb>
*/

/*
 * Probability that can get to the bonus objectives
 * */

pair<Px,Px> bonus_balls_hit(std::array<Dist,3> const& auto_dist,std::array<Dist,3> const& tele_dist){
	auto auto_dist_trun=min(9,sum(auto_dist));
	auto total=auto_dist_trun+sum(tele_dist);
	return make_pair(total>=29,total>=49);
}

std::ostream& operator<<(std::ostream& o,Robot_capabilities const& a){
	o<<"Robot_capabilities(";
	ROBOT_CAPABILITIES(SHOW)
	return o<<")";
}

/*Robot_capabilities operator/(Robot_capabilities a,size_t n){
	#define X(A,B) a.B/=n;
	ROBOT_CAPABILITIES(X)
	#undef X
	return a;
}

Robot_capabilities operator+(Robot_capabilities const& a,Robot_capabilities const& b){
	Robot_capabilities r;
	#define X(A,B) r.B=a.B+b.B;
	ROBOT_CAPABILITIES(X)
	#undef X
	return r;
}*/

Robot_capabilities mean(vector<Robot_capabilities> const& a){
	Robot_capabilities r;
	#define X(A,B) r.B=mean(mapf([](auto x){ return x.B; },a));
	ROBOT_CAPABILITIES(X)
	#undef X
	return r;
}

bool operator<(Robot_capabilities const& a,Robot_capabilities const& b){
	#define X(A,B) if(a.B<b.B) return 1; if(b.B<a.B) return 0;
	ROBOT_CAPABILITIES(X)
	#undef X
	return 0;
}

Px wheel_odds(Team team,std::vector<std::vector<Input_row>> const& alliance_results){
	//PRINT(team);
	//PRINT(alliance_results);

	//calculate number of time where the alliance had the option to go for a spin
	auto m=mapf(
		[team](auto x)->pair<bool,bool>{
			//returns success,available
			auto b=balls_towards_shield(x);
			if(b<TURN_THRESHOLD) return make_pair(0,0);
			auto by_this=group(
				[=](auto x){ return x.team==team; },
				x
			);
			auto other_robots=by_this[0];
			auto this_robot=by_this[1];
			assert(this_robot.size()==1);

			auto spins=sum(mapf([](auto a){ return a.wheel_spin; },other_robots));
			if(spins) return make_pair(0,0);

			return make_pair(this_robot[0].wheel_spin,1);
		},
		alliance_results
	);
	auto success=sum(firsts(m));
	auto available=sum(seconds(m));
	if(!available){
		return 0;
	}
	return (0.0+success)/available;
	nyi
}

Px wheel_color(Team team,std::vector<std::vector<Input_row>> const& alliance_results){
	//PRINT(team);
	//PRINT(alliance_results);

	//calculate number of time where the alliance had the option to go for a spin
	auto m=mapf(
		[team](auto x)->pair<bool,bool>{
			//returns success,available
			auto b=balls_towards_shield(x);
			if(b<COLOR_PICK_THRESHOLD) return make_pair(0,0);
			auto by_this=group(
				[=](auto x){ return x.team==team; },
				x
			);
			auto other_robots=by_this[0];
			auto this_robot=by_this[1];
			assert(this_robot.size()==1);

			auto colors=sum(mapf([](auto a){ return a.wheel_color; },other_robots));
			if(colors) return make_pair(0,0);

			return make_pair(this_robot[0].wheel_color,1);
		},
		alliance_results
	);
	auto success=sum(firsts(m));
	auto available=sum(seconds(m));
	if(!available){
		return 0;
	}
	return (0.0+success)/available;
	nyi
}

double assisted2(Team team,std::vector<std::vector<Input_row>> const& alliance_results){
	auto m=mapf(
		[=](auto x){
			auto others_could_take=filter(
				[=](auto elem){
					return elem.team!=team && (!elem.climbed || elem.climb_was_assisted);
				},
				x
			);
			auto available=others_could_take.size()>2;
			auto f=filter([=](auto x){ return x.team==team; },x);
			assert(f.size()==1);
			return make_pair(f[0].climb_assists==2,available);
		},
		alliance_results
	);
	auto available=sum(seconds(m));
	if(!available) return 0;
	return (0.0+sum(firsts(m)))/available;
}

double assisted1(Team team,std::vector<std::vector<Input_row>> const& alliance_results){
	auto m=mapf(
		[=](auto x){
			auto others_could_take=filter(
				[=](auto elem){
					return elem.team!=team && (!elem.climbed || elem.climb_was_assisted);
				},
				x
			);
			auto available=others_could_take.size();
			auto f=filter([=](auto x){ return x.team==team; },x);
			assert(f.size()==1);
			return make_pair(f[0].climb_assists==1,available);
		},
		alliance_results
	);
	auto available=sum(seconds(m));
	if(!available) return 0;
	return (0.0+sum(firsts(m)))/available;
}

Px park(std::vector<Input_row> const& a){
	auto available=filter([](auto x){ return !x.climbed; },a).size();
	if(!available) return 0;
	auto done=sum(mapf([](auto x){ return x.park; },a));
	return (done+0.0)/available;
}

Px balance(std::vector<Input_row> const& matches){
	unsigned attempts=0;
	unsigned successes=0;
	for(auto match:matches){
		if(match.climbed&&!match.climb_was_assisted){
			attempts+=1;
			if(match.balanced) successes+=1;
		}
	}
	if(attempts==0) return 0;
	return (0.0+successes)/attempts;
}

map<Team,Robot_capabilities> robot_capabilities(vector<Input_row> const& in){
	vector<vector<Input_row>> alliance_results=values(group(
		[](auto x){ return make_pair(x.match,x.alliance); },
		in
	));

	return to_map(mapf(
		[=](Team team)->pair<Team,Robot_capabilities>{
			auto f=filter(
				[team](auto x){ return x.team==team; },
				in
			);
			auto this_alliance=filter(
				[team](auto x){ return teams(x).count(team); },
				alliance_results
			);
			return make_pair(
				team,
				Robot_capabilities{
					mean_d(mapf([](auto x){ return x.auto_line; },f)),
					mean_d(mapf([](auto x){ return x.auto_low; },f)),
					mean_d(mapf([](auto x){ return x.auto_outer; },f)),
					mean_d(mapf([](auto x){ return x.auto_inner; },f)),
					mean_d(mapf([](auto x){ return x.tele_low; },f)),
					mean_d(mapf([](auto x){ return x.tele_outer; },f)),
					mean_d(mapf([](auto x){ return x.tele_inner; },f)),
					to_dist(mapf(
						[](auto x)->unsigned{ return x.auto_low+x.auto_outer+x.auto_inner; },
						f
					)),
					to_dist(mapf(
						[](auto x)->unsigned{ return x.tele_low+x.tele_outer+x.tele_inner; },
						f
					)),
					wheel_odds(team,this_alliance),
					wheel_color(team,this_alliance),
					mean_d(mapf([](auto x){ return x.climbed && !x.climb_was_assisted; },f)),//climb unassisted
					assisted2(team,this_alliance),
					assisted1(team,this_alliance),
					mean_d(mapf([](auto x){ return x.climbed && x.climb_was_assisted; },f)),//climb was assisted
					park(f),
					balance(f)
				}
			);
		},
		teams(in)
	));
}

auto wheel_spins(vector<Input_row> const& a){
	return sum(mapf([](auto x){ return x.wheel_spin; },a));
}

unsigned climb_assists_given(vector<Input_row> const& a){
	return sum(mapf([](auto x){ return x.climb_assists; },a));
}

unsigned climb_assists_rx(vector<Input_row> const& a){
	return sum(mapf([](auto x){ return x.climb_was_assisted; },a));
}

std::vector<Input_row> rand(std::vector<Input_row> const*){
	std::vector<Input_row> r;
	for(auto match:range(1,80)){
		std::vector<Input_row> match_data;
		for(auto alliance:alliances()){
			auto v=mapf(
				[=](auto _){
					(void)_;
					auto r=rand((Input_row*)0);
					r.match=Match(match);
					r.alliance=alliance;
					return r;
				},
				range(3)
			);
			while(wheel_spins(v)>1){
				v[rand()%3].wheel_spin=0;
			}

			while(climb_assists_given(v)>climb_assists_rx(v)){
				auto& x=v[rand()%3].climb_assists;
				if(x) x--;
			}

			while(climb_assists_rx(v)>climb_assists_given(v)){
				v[rand()%3].climb_was_assisted=0;
			}

			match_data|=v;
		}

		//make sure all three robots on the alliance are different
		while(teams(match_data).size()<6){
			match_data[rand()%6].team=rand((Team*)0);
		}

		r|=match_data;
	}
	return r;
}

void show(std::map<Team,Robot_capabilities> const& data){
	auto heading="Robot capabilities";
	string s=html(
		head(
			title(heading)
		)
		+body(
			h1(heading)
			+tag("table border",
				tr(
					th("Team")
					#define X(A,B) +th(""#B)
					ROBOT_CAPABILITIES(X)
					#undef X
				)+
				join(mapf(
					[](auto x){
						auto [team,data]=x;
						return tr(
							td(team)
							#define X(A,B) +td(data.B)
							ROBOT_CAPABILITIES(X)
							#undef X
						);
					},
					data
				))
			)
		)
	);
	write_file("robot_capabilities.html",s);
}

using Alliance_capabilities=std::array<Robot_capabilities,3>;

enum Climb_strategy{
	NONE,
	PARK,
	CLIMB_SELF,
	CLIMB_ASSISTED
};

vector<Climb_strategy> climb_strategies(){
	return vector<Climb_strategy>{
		Climb_strategy::NONE,
		Climb_strategy::PARK,
		Climb_strategy::CLIMB_SELF,
		Climb_strategy::CLIMB_ASSISTED
	};
}

std::ostream& operator<<(std::ostream& o,Climb_strategy const& a){
	#define X(A) if(a==Climb_strategy::A) return o<<""#A;
	X(NONE) X(PARK) X(CLIMB_SELF) X(CLIMB_ASSISTED)
	#undef X
	assert(0);
}

using Alliance_climb_strategy=std::array<Climb_strategy,3>;

vector<Alliance_climb_strategy> alliance_climb_strategies(){
	vector<Alliance_climb_strategy> r;
	for(auto a:climb_strategies()){
		for(auto b:climb_strategies()){
			for(auto c:climb_strategies()){
				r|=Alliance_climb_strategy{a,b,c};
			}
		}
	}
	return r;
}

struct Climb_capabilities{
	CLIMB_CAPABILITIES(INST)
};

std::ostream& operator<<(std::ostream& o,Climb_capabilities const& a){
	o<<"Climb_capabilities( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	CLIMB_CAPABILITIES(X)
	#undef X
	return o<<")";
}

using Alliance_climb_capabilities=std::array<Climb_capabilities,3>;

double climb_points(Alliance_climb_capabilities const& cap,Alliance_climb_strategy const& strat){
	auto z=zip(cap,strat);
	double park_points=0;
	vector<double> assists_available;
	vector<double> assists_req;
	double climbed=0;
	vector<Px> balancers;
	for(auto [cap1,strat1]:z){
		switch(strat1){
			case NONE:
				break;
			case PARK:
				park_points+=5*cap1.park;
				break;
			case CLIMB_SELF:
				climbed+=cap1.climb_unassisted;
				balancers|=cap1.balance;
				break;
			case CLIMB_ASSISTED:
				assists_req|=cap1.climb_was_assisted;
				break;
			default:
				assert(0);
		}
	}
	for(auto [a,b]:zip(
		reversed(sorted(assists_available)),
		reversed(sorted(assists_req))
	)){
		climbed+=mean(a,b);//geometric mean might be better
	}
	auto balance_points=balancers.empty()?0:(15*geomean(balancers));
	return park_points+climbed*25+balance_points;
}

double climb_points(Alliance_climb_capabilities const& cap){
	return max(mapf(
		[=](auto x){ return climb_points(cap,x); },
		alliance_climb_strategies()
	));
}

double expected_score(Alliance_capabilities const& a){
	//at some point might want to make this fancier and do things like scoring rates based on the amount of time
	//that might be used for the other activities
	//also, it would be good if had some idea of what the distribution of # of balls was so that could estimate
	//how likely it would be to get alliances to the different thresholds rather than just having a binary
	//yes/no on them.
	auto auto_line_points=5*sum(mapf([](auto x){ return x.auto_line; },a));

	auto auto_low=sum(mapf([](auto x){ return x.auto_low; },a));
	auto auto_outer=sum(mapf([](auto x){ return x.auto_outer; },a));
	auto auto_inner=sum(mapf([](auto x){ return x.auto_inner; },a));
	auto tele_low=sum(mapf([](auto x){ return x.tele_low; },a));
	auto tele_outer=sum(mapf([](auto x){ return x.tele_outer; },a));
	auto tele_inner=sum(mapf([](auto x){ return x.tele_inner; },a));

	auto ball_points=2*auto_low+4*auto_outer+6*auto_inner+tele_low+tele_outer*2+tele_inner*3;
	//auto auto_balls=auto_low+auto_outer+auto_inner;
	//auto tele_balls=tele_low+tele_outer+tele_inner;
	//auto balls_towards_shield=min(auto_balls,9)+tele_balls;

	//Could worry about whether or not the teleop # of balls is correlated with the auto number of balls
	//for example, if don't get certain shots off in auto then have the balls to shoot sooner in tele

	auto [p_spin_available,p_color_available]=bonus_balls_hit(
		mapf([](auto x){ return x.auto_ball_dist; },a),
		mapf([](auto x){ return x.tele_ball_dist; },a)
	);

	auto spin_points=[&]()->double{
		auto p=max(mapf([](auto x){ return x.wheel_spin; },a));
		return 10*p*p_spin_available;
	}();

	auto color_pick_points=[&]()->double{
		auto p=max(mapf([](auto x){ return x.wheel_color; },a));
		return 20*p*p_color_available;
	}();

	//hang points
	//hang strategies:
	//does each one want to hang?
	//for each of them, assume that they have y/n for attempt to do something for each team
	//then for each of those categories, choose between park/climb/get assisted (if available)
	//
	auto hang_points=climb_points(
		mapf(
			[](auto x){
				Climb_capabilities r;
				#define X(A,B) r.B=x.B;
				CLIMB_CAPABILITIES(X)
				#undef X
				return r;
			},
			a
		)
	);

	return auto_line_points+ball_points+spin_points+color_pick_points+hang_points;
}

using Picklist=vector<
	pair<
		pair<double,Team>,
		vector<pair<double,Team>>
	>
>;

void show_picklist(Team picker,Picklist const& a){
	auto heading="Team "+as_string(picker)+" Picklist";

	auto show_box=[](pair<double,Team> p)->string{
		return td(
			as_string(p.second)
			+"<br>"
			+tag(
				"small",
				tag("font color=grey",p.first)
			)
		);
	};

	auto s=html(
		head(title(heading))
		+body(
			h1(heading)+
			tag("table border",
				tr(
					tag("th colspan=2","First pick")+
					tag("th colspan=22","Second pick")
				)+
				tr(
					th("Rank")+
					th("Team")+
					join(mapf([](auto i){ return th(i); },range(1,23)))
				)+
				join(mapf(
					[=](auto p){
						auto [i,x]=p;
						auto [fp,second]=x;
						return tr(
							th(i)
							+show_box(fp)
							+join(mapf(show_box,take(22,second)))
						);
					},
					enumerate_from(1,take(15,a))
				))
			)
		)
	);
	write_file("picklist.html",s);
}

Picklist make_picklist(Team picking_team,map<Team,Robot_capabilities> a){
	auto picking_cap=a[picking_team];
	auto others=filter_keys(
		[=](auto k){
			return k!=picking_team;
		},
		a
	);
	
	auto x=reversed(sorted(mapf(
		[=](auto p){
			auto [t1,t1_cap]=p;
			(void)t1;
			Alliance_capabilities cap{picking_cap,t1_cap,{}};
			return make_pair(expected_score(cap),t1_cap);
		},
		others
	)));

	auto ex_cap=[&]()->Robot_capabilities{
		if(others.size()==0){
			return {};
		}
		if(others.size()>24){
			//take the 22nd-24th best robots as the example 3rd robot
			return mean(take(5,skip(22,seconds(x))));
		}
		//if fewer than 24, then just take the bottom 5.
		return mean(take(5,reversed(seconds(x))));
	}();

	cout<<"Example 3rd robot capabilities:\n";
	cout<<ex_cap<<"\n";

	auto first_picks=reversed(sorted(mapf(
		[=](auto p){
			auto [t1,t1_cap]=p;
			Alliance_capabilities cap{picking_cap,t1_cap,ex_cap};
			return make_pair(expected_score(cap),t1);
		},
		others
	)));

	//PRINT(first_picks);

	auto second_picks=to_map(mapf(
		[&](auto t1)->pair<Team,vector<pair<double,Team>>>{
			auto t1_cap=a[t1];
			auto left=without_key(t1,others);
			auto result=reversed(sorted(mapf(
				[=](auto x){
					auto [t2,t2_cap]=x;
					Alliance_capabilities cap{picking_cap,t1_cap,t2_cap};
					return make_pair(expected_score(cap),t2);
				},
				to_vec(left)
			)));
			return make_pair(t1,result);
		},
		to_vec(keys(others))
	));

	return mapf(
		[&](auto p){
			return make_pair(p,second_picks[p.second]);
		},
		first_picks
	);
}

void write_file(std::string const& path,std::vector<Input_row> const& data){
	ofstream o(path);
	vector<string> cols;
	#define X(A,B) cols|=""#B;
	INPUT_DATA(X)
	#undef X
	o<<join(",",cols)<<"\n";
	for(auto row:data){
		vector<string> items;
		#define X(A,B) items|=as_string(row.B);
		INPUT_DATA(X)
		#undef X
		o<<join(",",items)<<"\n";
	}
}

void run(Team team,std::optional<string> const& path){
	auto data=[=](){
		if(path) return read_csv(*path);

		//use random data if no path specified
		auto v=rand((std::vector<Input_row>*)0);
		write_file("example.csv",v);
		return read_csv("example.csv");
	}();
	sanity_check(data);

	auto rc=robot_capabilities(data);
	show(rc);

	//check that the target team is in the data.
	auto t=teams(data);
	if(!t.count(team)){
		cout<<"Warning: Did not find picking team ("<<team<<") in the data.  Adding zeros.\n";
		rc[team]={};
	}

	auto list=make_picklist(team,rc);
	show_picklist(team,list);
}

int main(int argc,char **argv){
	Team team{1425};
	auto set_team=[&](vector<string>& a)->int{
		assert(a.size());
		team=Team{stoi(a[0])};
		a=skip(1,a);
		return 0;
	};
	std::optional<std::string> path;
	auto set_path=[&](vector<string>& a)->int{
		assert(a.size());
		path=a[0];
		a=skip(1,a);
		return 0;
	};
	vector<tuple<
		const char *, //name
		const char *, //arg info
		const char *, //msg
		std::function<int(vector<string>&)>
	>> options{
		make_tuple(
			"--team","<TEAM NUMBER>","Create picklist for the given team number",
			std::function<int(vector<string>&)>(set_team)
		),
		make_tuple(
			"--file","<PATH>","Input data file",
			std::function<int(vector<string>&)>(set_path)
		)
	};
	
	auto help=[&](vector<string>&)->int{
		cout<<"Available arguments:\n";
		for(auto [arg,arg_info,msg,func]:options){
			(void)func;
			cout<<"\t"<<arg<<" "<<arg_info<<"\n";
			cout<<"\t\t"<<msg<<"\n";
		}
		exit(0);
	};
	options|=make_tuple(
		"--help","","Display this message",
		std::function<int(vector<string>&)>(help)
	);

	auto arg_list=args(argc,argv);
	arg_list=skip(1,arg_list); //skip name of the binary.

	while(arg_list.size()){
		bool found=0;
		for(auto [arg,arg_info,msg,func]:options){
			(void) arg_info;
			(void) msg;
			if(arg_list[0]==arg){
				found=1;
				arg_list=skip(1,arg_list);
				auto r=func(arg_list);
				if(r){
					cout<<"Failed.\n";
					return r;
				}
			}
		}
		if(!found){
			cerr<<"Unrecognized arg:"<<arg_list;
			return 1;
		}
	}

	run(team,path);
}
