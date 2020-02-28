#include<iostream>
#include<fstream>
#include<string.h>
#include "input_data.h"
#include "capabilities.h"
#include "game.h"

using namespace std;

bool contains(string needle,string haystack){
	return !!strstr(haystack.c_str(),needle.c_str());
}

vector<string> split(string s){
	vector<string> r;
	stringstream current;
	for(auto c:s){
		if(isblank(c)){
			if(current.str().size()){
				r|=current.str();
				current.str("");
			}
		}else{
			current<<c;
		}
	}
	if(current.str().size()){
		r|=current.str();
	}
	return r;
}

struct Args{
	string path;
	Team team;
};

void help(){
	cout<<"Arguments:\n";
	cout<<"--path\n";
	cout<<"\tThe path to the data file.\n";
	cout<<"--team\n";
	cout<<"\tTeam number to do the picking\n";
	cout<<"--help\n";
	cout<<"\tShow this message\n";
	exit(0);
}

Args parse_args(int argc,char **argv){
	(void)argc;
	Args r;
	r.team=Team{1425};
	for(auto at=argv+1;*at;++at){
		if(string(*at)=="--help"){
			help();
		}else if(string(*at)=="--team"){
			at++;
			assert(*at);
			r.team=Team(stoi(*at));
		}else if(string(*at)=="--path"){
			at++;
			assert(*at);
			r.path=*at;
		}else{
			cout<<"Invalid argument.\n";
			help();
		}
	}
	if(r.path==""){
		cout<<"Must specify input file\n";
		exit(1);
	}
	return r;
}

Dist point_dist(double value){
	Dist r;
	r.clear();
	r[value]=1;
	return r;
}

map<Team,Robot_capabilities> parse_viper(string path){
	ifstream f(path.c_str());
	string s;
	while(f.good() && !contains("Teleop Outer",s)) getline(f,s);
	
	map<Team,Robot_capabilities> r;
	while(f.good()){
		getline(f,s);
		if(s=="") continue;
		auto x=split(s);
		assert(x.size()==14);
		//Column names:
		//Team    OPR
		//OPRP    Auto Line       Climb   Auto    Auto Inner      Auto Outer      Auto Bottom     Teleop  Teleop Inner    Teleop Outer    Teleop Bottom   Penalty
		auto team=Team{stoi(x[0])};
		auto d=[&](int i){ return stod(x[i]); };
		auto opr=d(1);
		auto oprp=d(2);
		auto auto_line=d(3);
		auto climb=d(4)/25; //going to just assume that it's from hanging and not park, etc.
		auto auto_=d(5);
		auto auto_inner=d(6)/3;
		auto auto_outer=d(7)/2;
		auto auto_bottom=d(8);
		auto teleop=d(9);
		auto teleop_inner=d(10)/3;
		auto teleop_outer=d(11)/2;
		auto teleop_bottom=d(12);
		auto penalty=d(13);

		r[team]=Robot_capabilities{
			auto_line,
			auto_bottom,
			auto_outer,
			auto_inner,
			teleop_bottom,
			teleop_outer,
			teleop_inner,
			point_dist(auto_bottom+auto_outer+auto_inner),
			point_dist(teleop_bottom+teleop_outer+teleop_inner),
			.3, //wheel spin - assume it is done 30% of the time
			.3, //wheel color picking - assume done 30% of the time
			climb,
			0,//assist2
			0,//assist1
			0,//climb was assisted
			0,//park
			0//balance
		};

		(void)opr;
		(void)oprp;
		(void)auto_;
		(void)teleop;
		(void)penalty;
	}
	return r;
}

int main(int argc,char **argv){
	auto args=parse_args(argc,argv);
	auto rc=parse_viper(args.path);
	show(rc);
	auto list=make_picklist(args.team,rc);
	show_picklist(args.team,list);
	return 0;
}
