#include<functional>
#include "util.h"
#include "int_limited.h"
#include "dist.h"

using namespace std;

/*To do list:
 * 1) output example input file
 * 2) make the availability of the bonus objectives probabilistic - joint probability distribution
 * 3) different strategies about who is climbing effects time to do other things
 * 4) Improve quality of random robot match data
 * */

//start generic code

template<typename A,typename B>
vector<A> firsts(vector<pair<A,B>> const& a){
	return mapf([](auto x){ return x.first; },a);
}

template<typename T>
double mean_d(vector<T> v){
	assert(v.size());
	double x=sum(v);
	return x/v.size();
}

template<typename K,typename V>
std::map<K,V> to_map(std::vector<std::pair<K,V>> const& a){
	std::map<K,V> r;
	for(auto [k,v]:a) r[k]=v;
	return r;
}

template<typename A,typename B>
vector<pair<optional<A>,optional<B>>> zip_extend(vector<A> const& a,vector<B> const& b){
	auto a_at=begin(a);
	auto b_at=begin(b);
	auto a_end=end(a);
	auto b_end=end(b);
	vector<pair<optional<A>,optional<B>>> r;
	while(a_at!=a_end || b_at!=b_end){
		r|=make_pair(
			[=]()->optional<A>{
				if(a_at==a_end) return std::nullopt;
				return *a_at;
			}(),
			[=]()->optional<B>{
				if(b_at==b_end) return std::nullopt;
				return *b_at;
			}()
		);
		if(a_at!=a_end) a_at++;
		if(b_at!=b_end) b_at++;
	}
	return r;
}

template<typename T>
vector<pair<size_t,T>> enumerate(vector<T> const& a){
	vector<pair<size_t,T>> r;
	size_t i=0;
	for(auto const& elem:a){
		r|=make_pair(i++,elem);
	}
	return r;
}

int rand(int const*){ return rand(); }
bool rand(bool const*){ return rand()%2; }
unsigned rand(unsigned const*){ return rand(); }

template<typename T>
std::multiset<T> to_multiset(std::vector<T> const& a){
	return std::multiset<T>(begin(a),end(a));
}

template<typename T>
T max(multiset<T> const& a){
	assert(a.size());
	T r=*begin(a);
	for(auto elem:a){
		r=std::max(r,elem);
	}
	return r;
}

template<typename T,size_t N>
T max(std::array<T,N> const& a){
	static_assert(N);
	T r=a[0];
	for(auto i:range(size_t(1),N)){
		r=max(r,a[i]);
	}
	return r;
}

template<typename T>
T min(vector<T> const& a){
	assert(a.size());
	T r=*begin(a);
	for(auto elem:a){
		r=min(r,elem);
	}
	return r;
}

template<typename T>
auto dist(vector<T> const& a){
	auto m=to_multiset(a);
	map<T,size_t> r;
	for(auto elem:a){
		r[elem]=m.count(elem);
	}
	return reversed(sorted(mapf(
		[](auto a){
			return make_pair(a.second,a.first);
		},
		r
	)));
}

string take(size_t n,string s){
	return s.substr(0,n);
}

int parse(int const*,string const& s){
	return stoi(s);
}

bool parse(bool const*,string);
unsigned parse(unsigned const*,string const&);

template<typename Func,typename T>
vector<T> sort_by(vector<T> a,Func f){
	std::sort(begin(a),end(a),[&](auto x,auto y){ return f(x)<f(y); });
	return a;
}

template<typename T>
set<T> to_set(multiset<T> const& a){
	return std::set<T>(begin(a),end(a));
}

template<typename T>
std::vector<T> rand(std::vector<T> const*){
	return mapf(
		[](auto _){
			(void)_;
			return rand((T*)0);
		},
		range(rand()%100)
	);
}

template<typename Func,typename K,typename V>
map<K,V> filter_keys(Func f,map<K,V> a){
	map<K,V> r;
	for(auto [k,v]:a){
		if(f(k)){
			r[k]=v;
		}
	}
	return r;
}

template<typename K,typename V>
map<K,V> without_key(K const& k,map<K,V> a){
	a.erase(k);
	return a;
}

template<typename K,typename V>
set<K> keys(map<K,V> a){
	return to_set(mapf(
		[](auto p){ return p.first; },
		a
	));
}

double mean(double a,double b){
	return (a+b)/2;
}

//start program-specific code

using Team=Int_limited<1,1000*10>;

Team rand(Team const*){
	//make a smaller set than is theoretically allowed to make it likely to have the same ones come up
	return Team{2000+rand()%75};
}

using Match=Int_limited<1,200>; //qual match number

enum class Alliance{RED,BLUE};

std::ostream& operator<<(std::ostream& o,Alliance a){
	switch(a){
		case Alliance::RED:
			return o<<"RED";
		case Alliance::BLUE:
			return o<<"BLUE";
		default:
			assert(0);
	}
}

Alliance parse(Alliance const*,std::string const&)nyi

Alliance rand(Alliance const*){
	return (rand()%2)?Alliance::RED:Alliance::BLUE;
}

vector<Alliance> alliances(){ return {Alliance::RED,Alliance::BLUE}; }

using Climb_assists=Int_limited<0,2>;
using Balls=Int_limited<0,100>;
/*
input data:
team
*/
#define INPUT_DATA(X)\
	X(Team,team)\
	X(Match,match)\
	X(Alliance,alliance)\
	X(bool,auto_line)\
	X(Balls,auto_low)\
	X(Balls,auto_outer)\
	X(Balls,auto_inner)\
	X(Balls,tele_low)\
	X(Balls,tele_outer)\
	X(Balls,tele_inner)\
	X(bool,wheel_spin)\
	X(bool,wheel_color)\
	X(bool,climbed)\
	X(Climb_assists,climb_assists)\
	X(bool,climb_was_assisted)\
	X(bool,park)\

struct Input_row{
	INPUT_DATA(INST)
};

bool operator<(Input_row const& a,Input_row const& b){
	#define X(A,B) if(a.B<b.B) return 1; if(b.B<a.B) return 0;
	INPUT_DATA(X)
	#undef X
	return 0;
}

#define SHOW(A,B) o<<""#B<<":"<<a.B<<" ";

std::ostream& operator<<(std::ostream& o,Input_row const& a){
	o<<"Input_row( ";
	INPUT_DATA(SHOW)
	return o<<")";
}

set<Team> teams(vector<Input_row> const& a){
	return to_set(mapf([](auto x){ return x.team; },a));
}

Input_row rand(Input_row const*){
	Input_row r;
	#define X(A,B) r.B=rand((A*)0);
	INPUT_DATA(X)
	#undef X
	//put some things in to make it more realistic
	if(r.climbed) r.park=0;
	if(r.climb_was_assisted || r.park) r.climb_assists=Climb_assists(0);
	if(!r.climbed){
	       r.climb_was_assisted=0;
	       r.climb_assists=Climb_assists(0);
	}
	return r;
}

vector<Input_row> read_csv(string const& path){
	ifstream f(path);

	string s;
	getline(f,s);
	auto columns=split(s,',');
	vector<string> expected_columns;
	#define X(A,B) expected_columns|=""#B;
	INPUT_DATA(X)
	#undef X
	if(columns!=expected_columns){
		cout<<"Column mismatch:\n";
		for(auto [i,p]:enumerate(zip_extend(expected_columns,columns))){
			if(p.first!=p.second){
				cout<<i<<"\t"<<p.first<<"\t"<<p.second<<"\n";
			}
		}
		exit(1);
	}

	vector<Input_row> r;
	while(f.good()){
		string s;
		getline(f,s);
		auto data=split(s,',');
		assert(data.size()==columns.size());
		Input_row row;
		size_t i=0;
		#define X(A,B) row.B=parse((A*)0,data.at(i++));
		INPUT_DATA(X)
		#undef X
		r|=row;
	}
	return r;
}

static const int TURN_THRESHOLD=29;
static const int COLOR_PICK_THRESHOLD=49;

unsigned balls_scored_auto(Input_row const& a){
	return a.auto_low+a.auto_outer+a.auto_inner;
}

unsigned balls_scored_auto(std::vector<Input_row> const& a){
	return sum(MAP(balls_scored_auto,a));
}

unsigned balls_scored_tele(Input_row const& a){
	return a.tele_low+a.tele_outer+a.tele_inner;
}

unsigned balls_scored_tele(std::vector<Input_row> const& a){
	return sum(MAP(balls_scored_tele,a));
}

unsigned balls_scored(std::vector<Input_row> const& a){
	return balls_scored_auto(a)+balls_scored_tele(a);
}

auto balls_towards_shield(std::vector<Input_row> const& a){
	return min(balls_scored_auto(a),9)+balls_scored_tele(a);
}

void sanity_check(vector<Input_row> const& a){
	/*
	Sanity checks:
	-wheel turn when not enough balls
	-wheel colored when not enough balls
	-wheel colored when not turned
	-range of individual values
	-assists adding up on a team for climbs
	-not parking at the same time as climb
	-not having been assisted when parked
	-no giving assists if not climbed themselves
	-if more than one robot on an alliance credited with doing the same wheel item
	-the match numbers available look good

	also interesting to know:
	min/max/dist of each of the variables
	*/

	if(a.empty()){
		cout<<"No data found\n";
		exit(1);
	}

	cout<<"Seen data ranges:\n";
	#define X(A,B) {\
		auto values=mapf([](auto x){ return x.B; },a);\
		cout<<""#B<<":"<<min(values)<<" "<<max(values)<<" "<<take(50,as_string(dist(values)))<<"\n"; \
	}
	INPUT_DATA(X)
	#undef X
	cout<<"\n";

	auto matches=to_multiset(mapf([](auto x){ return x.match; },a));
	auto expected_range=range(1,max(matches)+1);
	for(auto i:expected_range){
		if(matches.count(Match(i))!=6){
			cout<<"Match "<<i<<": "<<matches.count(Match(i))<<" entries\n";
		}
	}

	//Check that the same robot does not appear more than once in the same match.
	for(auto [match,data]:group([](auto x){ return x.match; },a)){
		auto teams=to_multiset(mapf([](auto x){ return x.team; },data));
		for(auto team:to_set(teams)){
			if(teams.count(team)!=1){
				cout<<"Match "<<match<<" Team "<<team<<" appears more than once.\n";
			}
		}
	}

	auto by_alliance=group(
		[](auto x){ return make_pair(x.match,x.alliance); },
		a
	);

	for(auto [which,result]:sorted(to_vec(by_alliance))){
		auto marker=[=](){
			stringstream ss;
			ss<<"Match "<<which.first<<" Alliance "<<which.second<<": ";
			return ss.str();
		}();

		auto balls=balls_towards_shield(result);
		auto turns=sum(mapf([](auto x){ return x.wheel_spin; },result));
		auto color_picks=sum(mapf([](auto x){ return x.wheel_color; },result));

		if(turns && balls<TURN_THRESHOLD){
			cout<<marker<<"Too few balls scored ("<<balls<<") to score wheel turn.\n";
		}
		if(color_picks && balls<COLOR_PICK_THRESHOLD){
			cout<<marker<<"Too few balls scored ("<<balls<<") to score wheel color.\n";
		}
		if(turns>1){
			cout<<marker<<"More than one robot is given credit for scoring the wheel turn.\n";
		}

		auto assists_rx=sum(mapf([](auto x){ return x.climb_was_assisted; },result));
		auto assists_given=(unsigned)sum(mapf([](auto x){ return x.climb_assists; },result));
		if(assists_rx!=assists_given){
			cout<<marker<<"Climb assists given ("<<assists_given<<") is not equal to recieved ("<<assists_rx<<")\n";
		}
	}

	for(auto elem:sort_by(a,[](auto x){ return x.match; })){
		auto marker=[=](){
			stringstream ss;
			ss<<"Match "<<elem.match<<" Team "<<elem.team<<": ";
			return ss.str();
		}();

		if(elem.park && elem.climbed){
			cout<<marker<<"Parked but also marked as climbed.\n";
		}
		if(elem.park && elem.climb_assists){
			cout<<marker<<"Parked but also marked as assisting others in climb.\n";
		}
		if(elem.climb_assists && !elem.climbed){
			cout<<marker<<"Listed as assisted in climbs but did not climb itself.\n";
		}
		if(elem.climb_assists && elem.climb_was_assisted){
			cout<<marker<<"Listed as assisting in other climbs but own climb was assisted.\n";
		}
		if(!elem.climbed && elem.climb_was_assisted){
			cout<<marker<<"Listed as having been assisted in climb but did not climb.\n";
		}
	}
}

#define CLIMB_CAPABILITIES(X)\
	X(Px,climb_unassisted)\
	X(Px,assist2)\
	X(Px,assist1)\
	X(Px,climb_was_assisted)\
	X(Px,park)

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
					park(f) //park
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
	for(auto match:range(1,50)){
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
	for(auto [cap1,strat1]:z){
		switch(strat1){
			case NONE:
				break;
			case PARK:
				park_points+=5*cap1.park;
				break;
			case CLIMB_SELF:
				climbed+=cap1.climb_unassisted;
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
	return park_points+climbed*25;
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
	auto auto_balls=auto_low+auto_outer+auto_inner;
	auto tele_balls=tele_low+tele_outer+tele_inner;
	auto balls_towards_shield=min(auto_balls,9)+tele_balls;

	auto spin_points=[&]()->double{
		if(balls_towards_shield>TURN_THRESHOLD){
			//probability of getting it is equal to probability for the team on your alliance that is best at it
			auto p=max(mapf([](auto x){ return x.wheel_spin; },a));
			return 10*p;
		}
		return 0;
	}();

	auto color_pick_points=[&]()->double{
		if(balls_towards_shield>COLOR_PICK_THRESHOLD){
			auto p=max(mapf([](auto x){ return x.wheel_color; },a));
			return 20*p;
		}
		return 0;
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
			as_string(p.second)+"<br>"+tag("small",p.first)
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
					enumerate_from(1,take(16,a))
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

void run(Team team,std::optional<string> const& path){
	auto data=[=](){
		if(path) return read_csv(*path);

		//use random data if no path specified
		return rand((std::vector<Input_row>*)0);
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
			"--file","<PATH>","Create picklist for the given team number",
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
