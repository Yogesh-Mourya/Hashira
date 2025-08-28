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
    Rational operator*(const Rational& o) const { return Rational(num*o.num, den*o.den); }
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

// ---------- file helpers ----------
string read_file(const string& path){
    ifstream fin(path);
    if(!fin) throw runtime_error("Cannot open file: " + path);
    ostringstream ss; ss << fin.rdbuf();
    return ss.str();
}

// ---------- parse JSON (regex-based) ----------
struct PointRaw { long long x; int base; string y_str; };
struct CaseData { int n, k; vector<PointRaw> pts; };

CaseData parse_case_from_json(const string& js) {
    CaseData cd{0,0,{}};

    // "n" and "k"
    {
        regex n_re("\"n\"\\s*:\\s*([0-9]+)");
        regex k_re("\"k\"\\s*:\\s*([0-9]+)");
        smatch m;
        if (regex_search(js, m, n_re)) cd.n = stoi(m[1].str());
        else throw runtime_error("Missing n in keys");
        if (regex_search(js, m, k_re)) cd.k = stoi(m[1].str());
        else throw runtime_error("Missing k in keys");
    }

    // Numeric properties: "2": { "base":"15", "value":"..." }
    regex pt_re("\"([0-9]+)\"\\s*:\\s*\\{\\s*\"base\"\\s*:\\s*\"([0-9A-Za-z]+)\"\\s*,\\s*\"value\"\\s*:\\s*\"([0-9A-Za-z]+)\"\\s*\\}");
    for (sregex_iterator it(js.begin(), js.end(), pt_re), end; it != end; ++it) {
        PointRaw pr;
        pr.x     = stoll((*it)[1].str());
        pr.base  = stoi((*it)[2].str());
        pr.y_str = (*it)[3].str();
        cd.pts.push_back(pr);
    }
    if ((int)cd.pts.size() < cd.k)
        throw runtime_error("Not enough points to satisfy k");
    return cd;
}

// ---------- Lagrange c = f(0) using exactly k points ----------
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

int main(int argc, char** argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <json_file1> [json_file2 ...]\n";
        return 1;
    }

    try {
        for (int i = 1; i < argc; ++i) {
            string js = read_file(argv[i]);
            CaseData cd = parse_case_from_json(js);

            // decode all points
            vector<P> all;
            all.reserve(cd.pts.size());
            for (auto &pr : cd.pts) {
                P p{pr.x, parse_in_base(pr.y_str, pr.base)};
                all.push_back(p);
            }

            // choose first k by ascending x
            sort(all.begin(), all.end(), [](const P&a,const P&b){return a.x<b.x;});
            vector<P> used(all.begin(), all.begin() + cd.k);

            Rational c = lagrange_c_at_zero(used);

            // print result JSON (strings to be safe)
            if (c.den == 1) {
                std::string s = c.num.convert_to<string>();
                cout << "{\"c\":\"" << s << "\"}\n";
            } else {
                cpp_int q = c.num / c.den;
                cpp_int r = c.num % c.den;
                if (r == 0) {
                    std::string s = q.convert_to<string>();
                    cout << "{\"c\":\"" << s << "\"}\n";
                } else {
                    std::string ns = c.num.convert_to<string>();
                    std::string ds = c.den.convert_to<string>();
                    cout << "{\"c_num\":\"" << ns << "\",\"c_den\":\"" << ds << "\"}\n";
                }
            }
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
