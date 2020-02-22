#include<functional>
#include<cmath>
#include "util.h"
#include "int_limited.h"
#include "dist.h"
#include "input_data.h"
#include "capabilities.h"
#include "game.h"

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

//start program-specific code

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
