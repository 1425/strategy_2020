#include<fstream>
#include "../tba/db.h"
#include "../tba/tba.h"
#include "capabilities.h"
#include "util.h"

using namespace std;

template<typename... A>
std::ostream& operator<<(std::ostream& o,std::variant<A...> const& a){
        std::visit([&](auto &&elem){ o<<elem; },a);
        return o;
}

void print_r(unsigned indent,tba::Match const& a){
	nyi
	PRINT(indent);
	PRINT(a);
	/*indent_by(indent);
	indent++;
	cout<<"Match\n";
	#define X(A,B) print_r(indent,a.B);
	TBA_MATCH(X)
	#undef X*/
}

void print_r(unsigned indent,tba::Match_Score_Breakdown_2020_Alliance const& a){
	indent_by(indent);
	indent++;
	cout<<"Match_Score_Breakdown_2020_Alliance\n";
	#define X(A,B) indent_by(indent); cout<<""#B<<"("#A<<")\n"; print_r(indent+1,a.B);
	TBA_MATCH_SCORE_BREAKDOWN_2020_ALLIANCE(X)
	#undef X
}

void print_r(unsigned indent,tba::Match_Score_Breakdown_2020 const& a){
	indent_by(indent);
	indent++;
	cout<<"Match_Score_Breakdown_2020\n";
	#define X(A,B) indent_by(indent); cout<<""#B<<"\n"; print_r(indent+1,a.B);
	TBA_MATCH_SCORE_BREAKDOWN_2020(X)
	#undef X
}

template<typename A,typename B,typename C,typename D,typename E>
void print_r(unsigned indent,std::variant<A,B,C,D,E> const& a){
	if(std::holds_alternative<A>(a)) return print_r(indent,std::get<A>(a));
	if(std::holds_alternative<B>(a)) return print_r(indent,std::get<B>(a));
	if(std::holds_alternative<C>(a)) return print_r(indent,std::get<C>(a));
	if(std::holds_alternative<D>(a)) return print_r(indent,std::get<D>(a));
	if(std::holds_alternative<E>(a)) return print_r(indent,std::get<E>(a));
	assert(0);
}

template<typename A,typename B,typename C,typename D>
void print_r(unsigned,std::variant<A,B,C,D> const&)nyi

template<typename T>
void print_r(unsigned i,std::optional<T> const& a){
	if(a){
		return print_r(i,*a);
	}
	indent_by(i);
	cout<<"NULL\n";
}

#define INFO_ITEMS(X)\
	X(multiset<tba::Endgame>,endgame)\
	X(multiset<tba::Init_line>,init_line)\
	X(multiset<int>,alliance_balls_scored)

struct Info{
	INFO_ITEMS(INST)
};

std::ostream& operator<<(std::ostream& o,Info const& ){
	o<<"Info(...)";
	return o;
}

void print_r(unsigned indent,Info const& a){
	indent_by(indent);
	indent++;
	cout<<"Info\n";
	#define X(A,B) print_r(indent,a.B);
	INFO_ITEMS(X)
	#undef X
}

int main1(){
	std::ifstream f("../tba/auth_key");
	std::string tba_key;
	getline(f,tba_key);
	tba::Cached_fetcher cf{tba::Fetcher{tba::Nonempty_string{tba_key}},tba::Cache{}};

	tba::Event_key event_key{"2020isde1"};
	auto found=event_matches(cf,event_key);
	PRINT(found.size());
	//print_r(found);

	map<tba::Team_key,Info> info;

	for(auto match:found){
		/*#define X(A,B) cout<<""#B<<"("#A<<")"<<"\n"; print_r(1,match.B);
		TBA_MATCH(X)
		#undef X*/
		//nyi
		
		if(!match.score_breakdown) continue;
		/*for(auto [a,b]:zip(match.alliances,*match.score_breakdown)){
			PRINT(a);
			PRINT(b);
			nyi
		}*/

		auto analyze_alliance=[&](
			tba::Match_Alliance const& alliance,
			tba::Match_Score_Breakdown_2020_Alliance const& result
		){
			//print_r(0,alliance);
			//print_r(0,result);

			assert(alliance.team_keys.size()==3);
			auto& t1=info[alliance.team_keys[0]];
			auto& t2=info[alliance.team_keys[1]];
			auto& t3=info[alliance.team_keys[2]];

			t1.init_line|=result.initLineRobot1;
			t2.init_line|=result.initLineRobot2;
			t3.init_line|=result.initLineRobot3;

			t1.endgame|=result.endgameRobot1;
			t2.endgame|=result.endgameRobot2;
			t3.endgame|=result.endgameRobot3;

			auto auto_balls_scored=result.autoCellsBottom+result.autoCellsOuter+result.autoCellsInner;;
			auto teleop_balls_scored=result.teleopCellsBottom+result.teleopCellsOuter+result.teleopCellsInner;

			//this is only the number that could be towards advancing levels
			auto balls_scored=min(9,auto_balls_scored)+teleop_balls_scored;
			t1.alliance_balls_scored|=balls_scored;
			t2.alliance_balls_scored|=balls_scored;
			t3.alliance_balls_scored|=balls_scored;
		};

		analyze_alliance(
			match.alliances.red,
			get<tba::Match_Score_Breakdown_2020>(*match.score_breakdown).red
		);
		analyze_alliance(
			match.alliances.blue,
			get<tba::Match_Score_Breakdown_2020>(*match.score_breakdown).blue
		);
	}

	//PRINT(info.size());
	//print_r(info);

	auto m=mapf(
		[](auto p)->Robot_capabilities{
			auto [team,data]=p;
			PRINT(team);
			PRINT(data);
			/*			        X(Px,auto_line)\
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
*/
			nyi
		},
		info
	);
	print_lines(m);
	nyi
}

int main(){
	try{
		return main1();
	}catch(std::string const& s){
		std::cout<<"Caught: "<<s<<"\n";
	}
}
