#ifndef CAPABILITIES_H
#define CAPABILITIES_H

#include "dist.h"

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

std::pair<Px,Px> bonus_balls_hit(std::array<Dist,3> const& auto_dist,std::array<Dist,3> const& tele_dist){
	auto auto_dist_trun=min(9,sum(auto_dist));
	auto total=auto_dist_trun+sum(tele_dist);
	return std::make_pair(total>=29,total>=49);
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

Robot_capabilities mean(std::vector<Robot_capabilities> const& a){
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
		[team](auto x)->std::pair<bool,bool>{
			//returns success,available
			auto b=balls_towards_shield(x);
			if(b<TURN_THRESHOLD) return std::make_pair(0,0);
			auto by_this=group(
				[=](auto x){ return x.team==team; },
				x
			);
			auto other_robots=by_this[0];
			auto this_robot=by_this[1];
			assert(this_robot.size()==1);

			auto spins=sum(mapf([](auto a){ return a.wheel_spin; },other_robots));
			if(spins) return std::make_pair(0,0);

			return std::make_pair(this_robot[0].wheel_spin,1);
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
		[team](auto x)->std::pair<bool,bool>{
			//returns success,available
			auto b=balls_towards_shield(x);
			if(b<COLOR_PICK_THRESHOLD) return std::make_pair(0,0);
			auto by_this=group(
				[=](auto x){ return x.team==team; },
				x
			);
			auto other_robots=by_this[0];
			auto this_robot=by_this[1];
			assert(this_robot.size()==1);

			auto colors=sum(mapf([](auto a){ return a.wheel_color; },other_robots));
			if(colors) return std::make_pair(0,0);

			return std::make_pair(this_robot[0].wheel_color,1);
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
			return std::make_pair(f[0].climb_assists==2,available);
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
			return std::make_pair(f[0].climb_assists==1,available);
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

std::map<Team,Robot_capabilities> robot_capabilities(std::vector<Input_row> const& in){
	std::vector<std::vector<Input_row>> alliance_results=values(group(
		[](auto x){ return std::make_pair(x.match,x.alliance); },
		in
	));

	return to_map(mapf(
		[=](Team team)->std::pair<Team,Robot_capabilities>{
			auto f=filter(
				[team](auto x){ return x.team==team; },
				in
			);
			auto this_alliance=filter(
				[team](auto x){ return teams(x).count(team); },
				alliance_results
			);

			//This chunk of code exists because the scouting team doesn't think that they are
			//going to be able to see which balls go in which port for the first event
			auto auto_high=mean_d(mapf([](auto x){ return x.auto_high; },f));
			auto tele_high=mean_d(mapf([](auto x){ return x.tele_high; },f));
			static const Px inner_odds=.2;
			auto auto_outer=auto_high*(1-inner_odds);
			auto auto_inner=auto_high*inner_odds;
			auto tele_outer=tele_high*(1-inner_odds);
			auto tele_inner=tele_high*inner_odds;

			return std::make_pair(
				team,
				Robot_capabilities{
					mean_d(mapf([](auto x){ return x.auto_line; },f)),
					mean_d(mapf([](auto x){ return x.auto_low; },f)),
					auto_outer, //mean_d(mapf([](auto x){ return x.auto_outer; },f)),
					auto_inner, //mean_d(mapf([](auto x){ return x.auto_inner; },f)),
					mean_d(mapf([](auto x){ return x.tele_low; },f)),
					tele_outer, //mean_d(mapf([](auto x){ return x.tele_outer; },f)),
					tele_inner, //mean_d(mapf([](auto x){ return x.tele_inner; },f)),
					to_dist(mapf(
						[](auto x)->unsigned{
							//return x.auto_low+x.auto_outer+x.auto_inner;
							return x.auto_low+x.auto_high;
						},
						f
					)),
					to_dist(mapf(
						[](auto x)->unsigned{
							//return x.tele_low+x.tele_outer+x.tele_inner;
							return x.tele_low+x.tele_high;
						},
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

#endif
