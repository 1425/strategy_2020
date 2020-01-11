#ifndef INT_LIMITED_H
#define INT_LIMITED_H

template<int MIN,int MAX>
class Int_limited{
	int i;

	public:
	Int_limited():i(MIN){}

	explicit Int_limited(int value):i(value){
		assert(value>=MIN && value<=MAX);
	}

	operator int()const{ return i; }

	Int_limited operator--(int){
		auto r=*this;
		i--;
		assert(i>=MIN);
		return r;
	}
};

template<int MIN,int MAX>
Int_limited<MIN,MAX> parse(Int_limited<MIN,MAX> const*,std::string const&)nyi

template<int MIN,int MAX>
Int_limited<MIN,MAX> rand(Int_limited<MIN,MAX> const*){
	return Int_limited<MIN,MAX>(MIN+rand()%(MAX-MIN+1));
}

template<int MIN,int MAX>
int sum(std::vector<Int_limited<MIN,MAX>> const& a){
	int r=0;
	for(auto elem:a) r+=elem;
	return r;
}

#endif
