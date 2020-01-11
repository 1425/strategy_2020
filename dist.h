#ifndef DIST_H
#define DIST_H

using Px=double;//probability; should be 0-1.

struct Dist:public std::map<unsigned,Px>{
	//models a probability distribution with discrete outcomes.
	Dist(){
		(*this)[0]=1;
	}
};

Dist mean(std::vector<Dist> const& a){
	//just put in each of the items with a reduced probability.
	Dist r;
	if(a.size()){
		r.clear();
		for(auto d:a){
			for(auto [v,p]:d){
				r[v]+=p/a.size();
			}
		}
	}
	return r;
}

Dist operator+(Dist const& a,Dist const& b){
	PRINT(sum(values(a)));
	PRINT(sum(values(b)));

	
	Dist r;
	r.clear();
	for(auto [av,ap]:a){
		for(auto [bv,bp]:b){
			r[av+bv]+=ap*bp;
		}
	}
	PRINT(sum(values(r)));
	PRINT(r);
	return r;
}

Dist to_dist(std::vector<unsigned> const& a){
	Dist r;
	if(a.empty()){
		return r;
	}

	r.erase(0);
	Px per_count=1.0/a.size();
	for(auto elem:a){
		r[elem]+=per_count;
	}
	return r;
}

template<size_t N>
Dist sum(std::array<Dist,N>){
	nyi
}

Dist min(unsigned a,Dist b){
	Dist r;
	for(auto [val,p]:b){
		r[min(a,val)]+=p;
	}
	return r;
}

Px operator>=(Dist const& a,unsigned u){
	auto f=filter(
		[u](auto p){ return p.first>=u; },
		to_vec(a)
	);
	return sum(seconds(f));
}

#endif
