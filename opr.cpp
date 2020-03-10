#include "opr.h"
#include<cassert>
#include<iostream>
#include<map>
#include<vector>
#include<set>
#include "qr_solve.hpp"
#include "util.h"
#include "input_data.h"

using namespace std;

template<typename A,typename B,typename C>
std::vector<A> firsts(std::vector<std::tuple<A,B,C>> const& a){
	return mapf([](auto x){ return get<0>(x); },a);
}

template<typename A,typename B,typename C>
std::vector<B> seconds(std::vector<std::tuple<A,B,C>> const& a){
	return mapf([](auto x){ return get<1>(x); },a);
}

template<typename T>
std::vector<T>& operator|=(std::vector<T>& a,T t){
	a.push_back(t);
	return a;
}

template<typename T>
std::vector<T> flatten(std::vector<std::vector<T>> const& a){
	std::vector<T> r;
	for(auto elem:a){
		for(auto x:elem){
			r|=x;
		}
	}
	return r;
}

template<typename T>
bool contains(vector<T> const& a,T t){
	for(auto elem:a){
		if(t==elem){
			return 1;
		}
	}
	return 0;
}

/*template<typename T>
std::ostream& operator<<(std::ostream& o,std::vector<T> const& a){
	o<<"[ ";
	for(auto x:a){
		o<<x<<" ";
	}
	return o<<"]";
}*/

std::vector<std::pair<std::vector<Team>,int>> results(){
	std::vector<std::pair<std::vector<Team>,int>> r;
	for(int i=0;i<10;i++){
		r|=std::make_pair(
			std::vector<Team>{
				rand((Team*)0),
				rand((Team*)0),
				rand((Team*)0)
			},
			rand()%200
		);
	}
	return r;
}

std::vector<int> range(int n){
	std::vector<int> r;
	for(int i=0;i<n;i++){
		r|=i;
	}
	return r;
}

template<typename A,typename B>
std::vector<std::pair<A,B>> zip1(std::vector<A> const& a,B const* b){
	std::vector<std::pair<A,B>> r;
	for(auto [i,a1]:enumerate(a)){
		r|=std::make_pair(a1,b[i]);
	}
	return r;
}

std::vector<std::tuple<std::vector<Team>,std::vector<Team>,int>> demo_data(){
	auto alliance=[](){
		return vector{
			rand((Team*)0),
			rand((Team*)0),
			rand((Team*)0)
		};
	};
	return mapf(
		[=](auto i){
			(void)i;
			return make_tuple(
				alliance(),
				alliance(),
				rand()%200
			);
		},
		range(10)
	);
}

std::map<Team,double> solve(
	std::vector<std::tuple<std::vector<Team>,std::vector<Team>,int>> const& data
){
	auto teams=to_vec(to_set(flatten(firsts(data))|flatten(seconds(data))));
	map<Team,size_t> team_index=to_map(swap_pairs(enumerate(teams)));
	vector<double> a;//the matrix
	vector<double> b;
	for(auto [red,blue,red_margin]:data){
		for(auto team:teams){
			a|=[&](){
				if(contains(red,team)){
					return 1;
				}
				if(contains(blue,team)){
					return -1;
				}
				return 0;
			}();
		}
		b|=double(red_margin);
	}
	double *x2=svd_solve(teams.size(),data.size(),&(a[0]),&(b[0]));
	std::map<Team,double> r;
	for(auto [i,value]:zip1(range(teams.size()),x2)){
		r[teams[i]]=value;
	}
	return r;
}

std::map<Team,double> solve(std::vector<std::pair<std::vector<Team>,int>> const& x){
	std::map<Team,int> rename;
	auto teams=to_vec(to_set(flatten(firsts(x))));
	for(auto [i,team]:enumerate(teams)){
		rename[team]=i;
	}
	vector<double> a;
	vector<double> b;
	for(auto [teams1,points]:x){
		for(auto team:teams){
			if(contains(teams1,team)){
				a|=1.0;
			}else{
				a|=0.0;
			}
		}
		b|=double(points);
	}
	/*cout<<"a:\n";
	cout<<a.size()<<"\n";
	//cout<<a<<"\n";
	for(auto i:range(x.size())){
		for(auto j:range(teams.size())){
			auto n=i*x.size()+j;
			cout<<a[n]<<" ";
		}
		cout<<"\n";
	}
	cout<<"b:\n";
	cout<<b.size()<<"\n";
	cout<<b<<"\n";*/
	double *x2=svd_solve(teams.size(),x.size(),&(a[0]),&(b[0]));
	/*for(size_t i=0;i<teams.size();i++){
		cout<<i<<":"<<x2[i]<<"\n";
	}*/
	std::map<Team,double> r;
	for(auto [i,value]:zip1(range(teams.size()),x2)){
		r[teams[i]]=value;
	}
	return r;
}

int main(){
	{
		auto x=results();
		auto s=solve(x);
		//PRINT(s);
		print_lines(sorted(swap_pairs(to_vec(s))));
	}
	{
		auto d=demo_data();
		auto s=solve(d);
		print_lines(sorted(swap_pairs(to_vec(s))));
	}
	return 0;

	/*
	 *
	 * going to try to solve this first:
	 *  (a)       (b)
	 * [0 1]     [ 4]
	 * [2 0] x = [20]
	 *
	 * should result in 10,4
	 * */
	int m=2;
	int n=2;
	double a[]={0,20,2,0};
	double b[]={8,20};

	double *x2=svd_solve(m,n,a,b);
	assert(x2);

	for(int i=0;i<n;i++){
		cout<<i<<":"<<x2[i]<<"\n";
	}

	//delete[]a;
	//delete[]b;
	delete[]x2;
}
