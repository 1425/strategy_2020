#include<iostream>
#include<cmath>
#include<functional>
#include<unistd.h>
#include "../tba/data.h"
#include "input_data.h"
#include "../tba/db.h"
#include "../tba/tba.h"

using namespace std;

template<typename T>
T median(std::vector<T> const& a){
	assert(a.size());
	auto b=sorted(a);
	return b[b.size()/2];
}

template<typename T>
vector<T> flatten(std::vector<std::vector<T>> const& a){
	vector<T> r;
	for(auto elem:a){
		r|=elem;
	}
	return r;
}

//Given a team at an event, draw all of the charts for the matches that they have played so far
//After that works, rotate based on the alliance that they had been on at the time

struct Args{
	tba::Event_key event;
	Team team;
};

void help(){
	cout<<"usage: ./plots <EVENT_KEY> <TEAM>\n";
	exit(0);
}

Args parse_args(int argc,char **argv){
	if(argc!=3){
		help();
	}
	return Args{tba::Event_key{argv[1]},Team{atoi(argv[2])}};
}

void plot(std::vector<tba::Zebra_team> const& data){
	//This is going to feed the data out to python for the plotting.
	{
		ofstream f("plotter.py");
		f<<"import numpy as np\n";
		f<<"import matplotlib.pyplot as plt\n";
		//f<<"#a:"<<a<<"\n";
		for(auto a:data){
			f<<"xs=[]\n";
			auto python_literal=[](optional<double> x)->string{
				if(!x){
					return "None";
				}
				return as_string(x);
			};
			for(auto elem:a.xs){
				f<<"xs.append("<<python_literal(elem)<<")\n";
			}
			f<<"ys=[]\n";
			for(auto elem:a.ys){
				f<<"ys.append("<<python_literal(elem)<<")\n";
			}
			f<<"plt.plot(xs,ys)\n";
		}

		auto name="all";
		f<<"plt.savefig(\"fig_"<<name<<".png\")\n";
	}

	pid_t p=fork();
	if(p==0){
		auto r=system("python plotter.py");
		assert(r==0);
		exit(0);
	}
}

void plot(tba::Zebra_team const& a,string name){
	//This is going to feed the data out to python for the plotting.
	pid_t p=fork();
	if(p!=0) return;

	string py_name=[=](){
		stringstream ss;
		ss<<"plotter_"<<getpid()<<".py";
		return ss.str();
	}();

	{
		ofstream f(py_name);
		f<<"import numpy as np\n";
		f<<"import matplotlib.pyplot as plt\n";
		//f<<"#a:"<<a<<"\n";
		f<<"xs=[]\n";
		auto python_literal=[](optional<double> x)->string{
			if(!x){
				return "None";
			}
			return as_string(x);
		};
		for(auto elem:a.xs){
			f<<"xs.append("<<python_literal(elem)<<")\n";
		}
		f<<"ys=[]\n";
		for(auto elem:a.ys){
			f<<"ys.append("<<python_literal(elem)<<")\n";
		}
		f<<"plt.plot(xs,ys)\n";
		f<<"plt.savefig(\"fig_"<<name<<".png\")\n";
	}

	auto r=system( ("python "+py_name).c_str() );
	assert(r==0);
	exit(0);
}

tba::Team_key to_team_key(Team const& a){
	stringstream ss;
	ss<<"frc"<<a;
	return tba::Team_key{ss.str()};
}

bool operator==(tba::Team_key const& a,Team b){
	return a==to_team_key(b);
}

using Range=std::pair<double,double>;

std::pair<Range,Range> limits(std::vector<tba::Zebra_team> const& v){
	auto xs=non_null(flatten(mapf([](auto a){ return a.xs; },v)));
	auto ys=non_null(flatten(mapf([](auto a){ return a.ys; },v)));
	return make_pair(
		make_pair(min(xs),max(xs)),
		make_pair(min(ys),max(ys))
	);
}

using Point=std::pair<double,double>;
using Time=double;
using Datapoint=std::pair<Time,Point>;

template<typename T>
std::vector<std::pair<T,T>> pick_stride(unsigned stride,std::vector<T> const& a){
	//get all of the combinations that are a certain distance from each other.
	std::vector<std::pair<T,T>> r;
	for(size_t i=0;i+stride<a.size();i++){
		r|=make_pair(a[i],a[i+stride]);
	}
	return r;
}

void heatmap(std::function<double(int,int)> density,std::string name){
	auto p=fork();
	if(p!=0) return;

	auto plotfile="plot_heat_"+name+".py";
	{
		ofstream f(plotfile);
		f<<"import numpy as np\n";
		f<<"import matplotlib\n";
		f<<"import matplotlib.pyplot as plt\n";
		f<<"ylabels=[]\n";
		f<<"xlabels=[]\n";
		static const int WIDTH=53;
		static const int HEIGHT=30;
		for(auto y:range(HEIGHT)){
			f<<"ylabels.append("<<27-y<<")\n";
		}
		for(auto x:range(WIDTH)){
			f<<"xlabels.append("<<x<<")\n";
		}
		f<<"data=[\n";
		for(auto y:range(HEIGHT)){
			f<<"\t[";
			for(auto x:range(WIDTH)){
				auto v=min(50,density(x,27-y));
				f<<v<<",";
			}
			f<<"\t],\n";
		}
		f<<"\t]\n";
		f<<"fig,ax=plt.subplots()\n";
		f<<"im=ax.imshow(data)\n";
		f<<"ax.set_xticks(np.arange("<<WIDTH<<"))\n";
		f<<"ax.set_yticks(np.arange("<<HEIGHT<<"))\n";
		f<<"ax.set_xticklabels(xlabels)\n";
		f<<"ax.set_yticklabels(ylabels)\n";
		f<<"fig.tight_layout()\n";
		f<<"plt.savefig(\"heat_"<<name<<".png\")\n";
	}
	auto r=system((string()+"python "+plotfile).c_str());
	assert(r==0);
	exit(0);
}

template<typename A,typename B>
B max_seconds(std::vector<std::pair<A,B>> const& a){
	assert(a.size());
	B r=a[0].second;
	for(auto const& elem:a){
		r=max(r,elem.second);
	}
	return r;
}

void largest_jump(std::vector<double> const& time,tba::Zebra_team const& a,std::string const& name){
	assert(time.size()==a.xs.size());
	assert(time.size()==a.ys.size());

	vector<Datapoint> data;
	for(auto [t,x,y]:zip(time,a.xs,a.ys)){
		if(x){
			assert(y);
			data|=make_pair(t,make_pair(*x,*y));
		}else{
			assert(!y);
		}
	}

	map<pair<int,int>,unsigned> buckets;
	for(auto elem:data){
		buckets[make_pair(elem.second.first,elem.second.second)]++;
	}
	PRINT(buckets.size());
	heatmap(
		[&](int x,int y)->double{
			/*return filter(
				[=](auto elem){ return int(elem.second.first)==x && int(elem.second.second)==y; },
				data
			).size();*/
			return buckets[make_pair(x,y)];
		},
		name
	);

	optional<Datapoint> previous;
	vector<double> instant_speeds;
	for(auto elem:data){
		if(!previous){
			previous=elem;
			continue;
		}
		auto dt=elem.first-previous->first;
		assert(dt>0);
		auto dx=elem.second.first-previous->second.first;
		auto dy=elem.second.second-previous->second.second;
		auto dist=sqrt(dx*dx+dy*dy);
		auto speed=dist/dt;
		instant_speeds|=speed;
		previous=elem;
	}

	//interesting to know the shortest time to go distance x
	//make list of distances (every 5 feet?) and then go through increasing timescales till they all are filled in
	//double threshold=10;
	//going to just do this brute-force to start
	vector<pair<Datapoint,Datapoint>> r;
	//vector<pair<Point,Point>> r2;
	for(auto stride:range(size_t(1),data.size())){
		r|=pick_stride(stride,data);
	}
	vector<double> ts;
	vector<double> x1;
	vector<double> y1;

	for(auto i:range(data.size())){
		for(auto j:range(i,data.size())){
			auto p1=data[i];
			auto p2=data[j];
			ts|=p2.first-p1.first;
			x1|=p2.second.first-p1.second.first;
			y1|=p2.second.second-p2.second.second;
		}
	}

	using Distance=double;
	using Speed=double;
	vector<pair<Distance,Speed>> v;
	for(auto x:r){
		//PRINT(x);
		auto dt=x.second.first-x.first.first;
		assert(dt>0);
		auto dx=x.first.second.first-x.second.second.first;
		auto dy=x.first.second.second-x.second.second.second;
		auto dist=sqrt(dx*dx+dy*dy);
		//assert(dist>=0);
		//PRINT(dist);
		//PRINT(dt);
		auto speed=dist/dt;
		//assert(speed<=max(instant_speeds));
		v|=make_pair(dist,speed);
	}
	cout<<"\n"<<name<<"\n";
	cout<<"Distance (ft)\tSpeed (ft/s)\tTime (s)\n";
	auto g=group(
		[](auto x){
			(void)x;
			return 0;//int(x.first/5);
		},
		v
	);
	PRINT(g.size());
	for(auto dist:range(0,60,5)){
		auto m=filter([=](auto x){ return x.first>=dist && x.first<=dist+5; },v);
		if(m.size()){
			auto speed=max_seconds(m); //max(seconds(m));
			assert(speed>0);
			cout<<dist<<"\t\t"<<speed<<"\t\t"<<dist/speed<<"\n";
		}
	}

	//could make a histogram of these
	/*PRINT(min(instant_speeds));
	PRINT(max(instant_speeds));
	PRINT(mean(instant_speeds));
	PRINT(median(instant_speeds));*/
	//return max(instant_speeds);
}

tba::Zebra_team flip_field(tba::Zebra_team a){
	a.xs=mapf(
		[](auto x)->optional<double>{
			if(x){
				return 52+5.25/12-*x;
			}
			return x;
		},
		a.xs
	);
	a.ys=mapf(
		[](auto y)->optional<double>{
			if(y){
				return 26+11.25/12-*y;
			}
			return y;
		},
		a.ys
	);
	return a;
}

tba::Zebra_team find_team(Team team,tba::Zebra const& data){
	for(auto x:data.alliances.red){
		if(x.team_key==team){
			return x;
		}
	}
	for(auto x:data.alliances.blue){
		if(x.team_key==team){
			//Take advantage of the rotational symmetry to normalize to being red.
			return flip_field(x);
		}
	}
	assert(0);
}

Team to_team(tba::Team_key a){
	return Team{atoi(a.str().c_str()+3)};
}

int main1(int argc,char **argv){
	auto args=parse_args(argc,argv);
	std::ifstream f("../tba/auth_key");
	std::string tba_key;
	getline(f,tba_key);
	tba::Cached_fetcher cf{tba::Fetcher{tba::Nonempty_string{tba_key}},tba::Cache{}};
	static const auto YEAR=tba::Year{2020};

	auto e=event_teams_keys(cf,tba::Event_key{"2020orwil"});

	for(auto t:e){
		std::vector<tba::Zebra_team> paths;
		for(auto match_key:team_matches_year_keys(cf,t,YEAR)){
			auto data=zebra_motionworks(cf,match_key);
			if(!data) continue;
			paths|=find_team(to_team(t),*data);
		}
		using Mode=int;//0=auto,1=tele main, 2=endgame
		map<Mode,map<pair<int,int>,unsigned>> m;
		for(auto elem:paths){
			for(auto [i,point]:enumerate(zip(elem.xs,elem.ys))){
				if(point.first){
					auto mode=[=](){
						if(i<15*10) return 0;
						if(i<(15+135-30)*10) return 1;
						return 2;
					}();
					m[mode][make_pair(*point.first,*point.second)]++;
				}
			}
		}
		for(auto [mode,info]:m){
			heatmap(
				[&](int x,int y){ return info[make_pair(x,y)]; },
				as_string(t)+"_"+as_string(mode)
			);
		}
	}

	auto matches=team_matches_year_keys(cf,to_team_key(args.team),YEAR);
	//matches={tba::Match_key{"2020orore_f1m1"}};

	vector<tba::Zebra_team> paths;
	for(auto match_key:matches){
		auto data=zebra_motionworks(cf,match_key);
		if(!data) continue;
		/*auto find_path=[&](){
			for(auto x:data->alliances.red){
				if(x.team_key==args.team){
					return x;
				}
			}
			for(auto x:data->alliances.blue){
				if(x.team_key==args.team){
					return flip_field(x);
				}
			}
			assert(0);
		};*/
		auto p=find_team(args.team,*data);
		plot(p,match_key.get());
		paths|=p;

		largest_jump(data->times,p,match_key.get());
		//cout<<largest_jump(data->times,p)<<"\n";

		//TODO: Normalize alliance
		//TODO: Make it so that can recognize trench bots
		//Measure top speeds for different robots
		//Measure fastest speed for a robot to have netted a certain distance
		//Look for outliers in the data to see what the worst jumpiness is
		//(difference in location in a single timestep)
		//Look at where the starting locations are to see where like to line up
		//Put in heat charts for auto vs tele
	}

	plot(paths);
	PRINT(limits(paths));

	vector<pair<int,int>> combined;
	for(auto zt:paths){
		for(auto [x,y]:zip(zt.xs,zt.ys)){
			if(x){
				combined|=make_pair(int(*x),int(*y));
			}
		}
	}
	map<pair<int,int>,unsigned> by_loc;
	for(auto elem:combined){
		by_loc[elem]++;
	}
	heatmap(
		[&](int x,int y)->double{
			return by_loc[make_pair(x,y)]; //flipping here because of the direction the plot goes
		},
		"overall"
	);
	return 0;
}

int main(int argc,char **argv){
	try{
		return main1(argc,argv);
	}catch(tba::Decode_error const& a){
		cout<<"Caught: "<<a<<"\n";
		return 1;
	}
}
