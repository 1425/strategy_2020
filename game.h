#ifndef GAME_H
#define GAME_H

#include "capabilities.h"

Alliance operator-(Alliance a){
	switch(a){
		case Alliance::RED:
			return Alliance::BLUE;
		case Alliance::BLUE:
			return Alliance::RED;
		default:
			assert(0);
	}
}

using Alliance_station_number=Int_limited<1,3>;

#define ALLIANCE_STATION_ITEMS(X)\
	X(Alliance,alliance)\
	X(Alliance_station_number,number)

struct Alliance_station{
	ALLIANCE_STATION_ITEMS(INST)
};

bool operator==(Alliance_station const& a,Alliance_station const& b){
	#define X(A,B) if(a.B!=b.B) return 0;
	ALLIANCE_STATION_ITEMS(X)
	#undef X
	return 1;
}

bool operator<(Alliance_station const& a,Alliance_station const& b){
	#define X(A,B) if(a.B<b.B) return 1; if(b.B<a.B) return 0;
	ALLIANCE_STATION_ITEMS(X)
	#undef X
	return 0;
}

std::ostream& operator<<(std::ostream& o,Alliance_station const& a){
	o<<"(";
	ALLIANCE_STATION_ITEMS(SHOW)
	return o<<")";
}

std::vector<Alliance_station> options(Alliance_station const*){
	std::vector<Alliance_station> r;
	for(auto a:options((Alliance*)0)){
		for(auto b:options((Alliance_station_number*)0)){
			r|=Alliance_station{a,b};
		}
	}
	return r;
}

using Alliance_capabilities=std::array<Robot_capabilities,3>;

enum class Climb_strategy{
	NONE,
	PARK,
	CLIMB_SELF,
	CLIMB_ASSISTED
};

std::vector<Climb_strategy> climb_strategies(){
	return std::vector<Climb_strategy>{
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

std::vector<Alliance_climb_strategy> alliance_climb_strategies(){
	std::vector<Alliance_climb_strategy> r;
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
	std::vector<double> assists_available;
	std::vector<double> assists_req;
	double climbed=0;
	std::vector<Px> balancers;
	for(auto [cap1,strat1]:z){
		switch(strat1){
			case Climb_strategy::NONE:
				break;
			case Climb_strategy::PARK:
				park_points+=5*cap1.park;
				break;
			case Climb_strategy::CLIMB_SELF:
				climbed+=cap1.climb_unassisted;
				balancers|=cap1.balance;
				break;
			case Climb_strategy::CLIMB_ASSISTED:
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

using Alliance_strategy=Alliance_climb_strategy;

double expected_score(Alliance_capabilities const& a,Alliance_strategy const& strat){
	//at some point might want to make this fancier and do things like scoring rates based on the amount of time
	//that might be used for the other activities
	//also, it would be good if had some idea of what the distribution of # of balls was so that could estimate
	//how likely it would be to get alliances to the different thresholds rather than just having a binary
	//yes/no on them.
	auto auto_line_points=5*sum(mapf([](auto x){ return x.auto_line; },a));

	auto auto_low=sum(mapf([](auto x){ return x.auto_low; },a));
	auto auto_outer=sum(mapf([](auto x){ return x.auto_outer; },a));
	auto auto_inner=sum(mapf([](auto x){ return x.auto_inner; },a));

	auto sum_scaled=[=](std::array<double,3> a)->double{
		double r=0;
		for(auto [value,climb]:zip(a,strat)){
			if(climb==Climb_strategy::NONE){
				r+=value;
			}else{
				static const double SCALE_FACTOR=(teleop_length()-endgame_length()+0.0)/teleop_length();
				r+=value*SCALE_FACTOR;
			}
		}
		return r;
	};
	auto tele_low=sum_scaled(mapf([](auto x){ return x.tele_low; },a));
	auto tele_outer=sum_scaled(mapf([](auto x){ return x.tele_outer; },a));
	auto tele_inner=sum_scaled(mapf([](auto x){ return x.tele_inner; },a));

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
		),
		strat
	);

	return auto_line_points+ball_points+spin_points+color_pick_points+hang_points;
}

double expected_score(Alliance_capabilities const& a){
	return max(mapf(
		[=](auto x){ return expected_score(a,x); },
		alliance_climb_strategies()
	));
}

#endif
