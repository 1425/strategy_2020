#include<functional>
#include "util.h"

using namespace std;

//start generic code

template<int MIN,int MAX>
class Int_limited{
	int i;

	public:
	Int_limited():i(MIN){}

	explicit Int_limited(int value):i(value){
		assert(value>=MIN && value<=MAX);
	}

	operator int()const{ return i; }

	//bool operator<(Int_limited)const;
};

template<int MIN,int MAX>
Int_limited<MIN,MAX> parse(Int_limited<MIN,MAX> const*,std::string const&)nyi

template<int MIN,int MAX>
Int_limited<MIN,MAX> rand(Int_limited<MIN,MAX> const*){
	return Int_limited<MIN,MAX>(MIN+rand()%(MAX-MIN+1));
}

template<int MIN,int MAX>
int sum(vector<Int_limited<MIN,MAX>> const& a){
	int r=0;
	for(auto elem:a) r+=elem;
	return r;
}

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

//start program-specific code

using Team=Int_limited<1,1000*10>;
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
	return r;
}

int parse(int const*,string const& s){
	return stoi(s);
}

bool parse(bool const*,string);
unsigned parse(unsigned const*,string const&);

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

using Px=double;//probability; should be 0-1.

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
			//cout<<"Match "<<i<<": "<<matches.count(Match(i))<<" robots\n";
		}
	}

	auto by_alliance=group(
		[](auto x){ return make_pair(x.match,x.alliance); },
		a
	);

	for(auto [which,result]:by_alliance){
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

	for(auto elem:a){
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

#define ROBOT_CAPABILITIES(X)\
	X(Px,auto_line)\
	X(double,auto_low)\
	X(double,auto_outer)\
	X(double,auto_inner)\
	X(double,tele_low)\
	X(double,tele_outer)\
	X(double,tele_inner)\
	X(Px,wheel_spin)\
	X(Px,wheel_color)\
	X(Px,climb_unassisted)\
	X(Px,assist2)\
	X(Px,assist1)\
	X(Px,climb_was_assisted)\
	X(Px,park)

struct Robot_capabilities{
	ROBOT_CAPABILITIES(INST)
};

std::ostream& operator<<(std::ostream& o,Robot_capabilities const& a){
	o<<"Robot_capabilities(";
	ROBOT_CAPABILITIES(SHOW)
	nyi
}

Px wheel_odds(Team team,std::vector<std::vector<Input_row>> const& alliance_results){
	PRINT(team);
	PRINT(alliance_results);

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
	PRINT(team);
	PRINT(alliance_results);

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
					wheel_odds(team,this_alliance),
					wheel_color(team,this_alliance),
					0,//climb unassisted
					0,//assisted2
					0,//assisted1
					0,//climb was assisted
					0//park
				}
			);
		},
		teams(in)
	));
}

template<typename T>
std::vector<T> rand(std::vector<T> const*){
	return mapf(
		[](auto _){
			(void)_;
			return rand((T*)0);
		},
		range(rand()%20)
	);
}

void show(std::vector<Robot_capabilities> const&){
	nyi
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
		cout<<"Did not find picking team in the data.  Aborting.\n";
		exit(1);
	}

	nyi
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
