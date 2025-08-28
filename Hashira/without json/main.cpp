#include <bits/stdc++.h>
#include <boost/multiprecision/cpp_int.hpp>
using namespace std;
using boost::multiprecision::cpp_int;

// ---------- exact rational ----------
struct Rational {
    cpp_int num, den; // den>0
    static cpp_int abs_cpp(const cpp_int& x){ return x>=0?x:-x; }
    static cpp_int gcd_cpp(cpp_int a, cpp_int b){
        a = abs_cpp(a); b = abs_cpp(b);
        while(b!=0){ cpp_int r=a%b; a=b; b=r; }
        return a==0? cpp_int(1): a;
    }
    Rational(cpp_int n=0, cpp_int d=1): num(n), den(d){ normalize(); }
    void normalize(){
        if(den==0) throw runtime_error("Zero denominator");
        if(den<0){ den=-den; num=-num; }
        cpp_int g=gcd_cpp(num,den); num/=g; den/=g;
    }
    Rational operator+(const Rational& o) const { return Rational(num*o.den + o.num*den, den*o.den); }
    Rational operator-(const Rational& o) const { return Rational(num*o.den - o.num*den, den*o.den); }
    Rational operator*(const Rational& o) const { return Rational(num*o.num, den*o.den); }
    Rational operator/(const Rational& o) const {
        if(o.num==0) throw runtime_error("Divide by zero");
        return Rational(num*o.den, den*o.num);
    }
};

// ---------- base decode ----------
int digit_of(char c){
    if('0'<=c && c<='9') return c-'0';
    if('a'<=c && c<='z') return 10 + (c-'a');
    if('A'<=c && c<='Z') return 10 + (c-'A');
    return -1;
}
cpp_int parse_in_base(const string& s, int base){
    if(base<2 || base>36) throw runtime_error("Unsupported base");
    cpp_int v=0;
    for(char c: s){
        if(c=='_' || c==' ') continue;
        int d=digit_of(c);
        if(d<0 || d>=base) throw runtime_error(string("Bad digit '")+c+"' for base "+to_string(base));
        v = v*base + d;
    }
    return v;
}

// ---------- compute c = f(0) via Lagrange using k points ----------
struct P { long long x; cpp_int y; };

Rational lagrange_c_at_zero(vector<P> pts){ // pts size = k
    sort(pts.begin(), pts.end(), [](const P&a,const P&b){return a.x<b.x;});
    Rational s(0,1);
    int k=(int)pts.size();
    for(int i=0;i<k;i++){
        Rational term(pts[i].y, 1);
        for(int j=0;j<k;j++) if(j!=i){
            cpp_int num = -pts[j].x;           // -x_j
            cpp_int den =  pts[i].x - pts[j].x; // (x_i - x_j)
            term = term * Rational(num, den);
        }
        s = s + term;
    }
    return s;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T; 
    if(!(cin>>T)){ cerr<<"Failed to read T\n"; return 1; }
    while(T--){
        int n,k; 
        cin>>n>>k;
        vector<P> all(n);
        for(int i=0;i<n;i++){
            long long x; int base; string val;
            cin>>x>>base>>val;
            all[i].x = x;
            all[i].y = parse_in_base(val, base);
        }
        sort(all.begin(), all.end(), [](const P&a,const P&b){return a.x<b.x;});
        vector<P> used(all.begin(), all.begin()+k);
        Rational c = lagrange_c_at_zero(used);
        if(c.den==1) cout<< c.num <<"\n";
        else {
            cpp_int q=c.num/c.den, r=c.num%c.den;
            if(r==0) cout<< q <<"\n";
            else     cout<< c.num <<"/"<< c.den <<"\n";
        }
    }
    return 0;
}
