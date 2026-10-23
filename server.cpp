// Cyber Detective C++ HTTP backend (C++17, Windows/Linux/macOS sockets; Render-ready)
// Educational demo: password hash below is NOT suitable for production authentication.
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using socket_t = SOCKET;
#define CLOSESOCKET closesocket
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using socket_t = int;
#define CLOSESOCKET close
#endif
using namespace std;
namespace fs = std::filesystem;
const int PORT = [](){
    const char* value = std::getenv("PORT");
    if (value && *value) { try { int p = std::stoi(value); if (p > 0 && p <= 65535) return p; } catch (...) {} }
    return 8787;
}();
const string DATA_DIR = [](){ const char* value = std::getenv("DATA_DIR"); return (value && *value) ? string(value) : string("data"); }();
string trim(const string& s){size_t a=s.find_first_not_of(" \r\n\t"),b=s.find_last_not_of(" \r\n\t");return a==string::npos?"":s.substr(a,b-a+1);}
string lower(string s){for(char& c:s)c=static_cast<char>(tolower(static_cast<unsigned char>(c)));return s;}
string decode(const string& s){string o;for(size_t i=0;i<s.size();++i){if(s[i]=='+')o+=' ';else if(s[i]=='%'&&i+2<s.size()){try{o+=static_cast<char>(stoi(s.substr(i+1,2),nullptr,16));i+=2;}catch(...){o+=s[i];}}else o+=s[i];}return o;}
map<string,string> form(const string& s){map<string,string> m;size_t p=0;while(p<s.size()){size_t e=s.find('&',p);string part=s.substr(p,e==string::npos?string::npos:e-p);size_t q=part.find('=');m[decode(part.substr(0,q))]=q==string::npos?"":decode(part.substr(q+1));if(e==string::npos)break;p=e+1;}return m;}
string jsonEscape(const string& s){string o;for(unsigned char c:s){switch(c){case '"':o+="\\\"";break;case '\\':o+="\\\\";break;case '\n':o+="\\n";break;case '\r':o+="\\r";break;case '\t':o+="\\t";break;default:if(c>=32)o+=static_cast<char>(c);}}return o;}
string response(const string& body,const string& type="application/json; charset=utf-8",int status=200){string label=status==200?"OK":status==201?"Created":status==400?"Bad Request":status==401?"Unauthorized":status==404?"Not Found":status==409?"Conflict":status==500?"Internal Server Error":"Error";ostringstream o;o<<"HTTP/1.1 "<<status<<" "<<label<<"\r\nContent-Type: "<<type<<"\r\nContent-Length: "<<body.size()<<"\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Headers: Content-Type\r\nAccess-Control-Allow-Methods: GET, POST, OPTIONS\r\nConnection: close\r\nX-Content-Type-Options: nosniff\r\n\r\n"<<body;return o.str();}
string ok(const string& msg){return "{\"ok\":true,\"message\":\""+jsonEscape(msg)+"\"}";}string err(const string& msg){return "{\"ok\":false,\"message\":\""+jsonEscape(msg)+"\"}";}
// Simple deterministic hash for a classroom demo only. Use a proper password KDF in real systems.
string passwordHash(const string& s){uint64_t h=1469598103934665603ULL;for(unsigned char c:s){h^=c;h*=1099511628211ULL;}ostringstream o;o<<hex<<h;return o.str();}
bool validUser(const string& u){if(u.size()<3||u.size()>24)return false;return all_of(u.begin(),u.end(),[](unsigned char c){return isalnum(c)||c=='_'||c=='-';});}
fs::path accountPath(){return fs::path(DATA_DIR)/"accounts.tsv";}fs::path progressPath(const string& u){return fs::path(DATA_DIR)/("progress_"+u+".json");}
map<string,string> readAccounts(){map<string,string> a;ifstream f(accountPath());string u,h;while(f>>u>>h)a[u]=h;return a;}
void writeAccounts(const map<string,string>& a){ofstream f(accountPath(),ios::trunc);for(auto& x:a)f<<x.first<<' '<<x.second<<'\n';}
string getValue(const string& json,const string& key){string needle="\""+key+"\"";size_t p=json.find(needle);if(p==string::npos)return "";p=json.find(':',p+needle.size());if(p==string::npos)return "";++p;while(p<json.size()&&isspace(static_cast<unsigned char>(json[p])))++p;if(p<json.size()&&json[p]=='"'){size_t e=json.find('"',p+1);return e==string::npos?"":json.substr(p+1,e-p-1);}size_t e=json.find_first_of(",}\r\n",p);return trim(json.substr(p,e==string::npos?string::npos:e-p));}
string handle(const string& method,const string& target,const string& body){string path=target.substr(0,target.find('?'));if(method=="OPTIONS")return response("", "text/plain",200);if(path=="/api/health")return response("{\"ok\":true,\"service\":\"Cyber Detective C++ backend\"}");
 if(method=="POST"&&(path=="/api/register"||path=="/api/login")){auto f=form(body);string u=lower(trim(f["username"])),p=f["password"];if(!validUser(u)||p.size()<4||p.size()>128)return response(err("Username must be 3-24 letters/numbers/_/- and password at least 4 characters."),"application/json; charset=utf-8",400);auto a=readAccounts();if(path=="/api/register"){if(a.count(u))return response(err("Username already exists. Log in instead."),"application/json; charset=utf-8",409);a[u]=passwordHash(p);writeAccounts(a);ofstream pf(progressPath(u));pf<<"{\"name\":\""<<jsonEscape(u)<<"\",\"score\":0,\"completed\":[],\"correct\":0,\"answered\":0,\"history\":[]}";return response(ok("Profile created."),"application/json; charset=utf-8",201);}if(!a.count(u)||a[u]!=passwordHash(p))return response(err("Incorrect username or password."),"application/json; charset=utf-8",401);return response("{\"ok\":true,\"username\":\""+jsonEscape(u)+"\"}");}
 if(method=="GET"&&path=="/api/progress"){auto q=target.find('?');if(q==string::npos)return response(err("Username is required."),"application/json; charset=utf-8",400);auto f=form(target.substr(q+1));string u=lower(f["username"]);if(!validUser(u)||!readAccounts().count(u))return response(err("Profile not found."),"application/json; charset=utf-8",404);ifstream in(progressPath(u));if(!in)return response("{\"name\":\""+jsonEscape(u)+"\",\"score\":0,\"completed\":[],\"correct\":0,\"answered\":0,\"history\":[]}");ostringstream s;s<<in.rdbuf();string data=s.str();return response(data.empty()?"{}":data);}
 if(method=="POST"&&path=="/api/progress"){auto f=form(body);string u=lower(f["username"]),data=f["data"];if(!validUser(u)||!readAccounts().count(u))return response(err("Please log in again."),"application/json; charset=utf-8",401);if(data.size()>200000)return response(err("Progress payload too large."),"application/json; charset=utf-8",400);if(data.empty()||data.front()!='{'||data.back()!='}')return response(err("Invalid progress data."),"application/json; charset=utf-8",400);ofstream out(progressPath(u),ios::trunc);if(!out)return response(err("Could not save progress."),"application/json; charset=utf-8",500);out<<data;out.close();return response(ok("Progress saved to C++ backend."));}
 if(method=="GET"&&path=="/api/leaderboard"){vector<pair<string,int>> rows;for(auto& entry:readAccounts()){ifstream in(progressPath(entry.first));ostringstream ss;ss<<in.rdbuf();string s=ss.str();int score=0;try{score=stoi(getValue(s,"score"));}catch(...){}rows.push_back({entry.first,score});}sort(rows.begin(),rows.end(),[](auto&a,auto&b){return a.second!=b.second?a.second>b.second:a.first<b.first;});ostringstream out;out<<"{\"entries\":[";for(size_t i=0;i<rows.size();++i){if(i)out<<',';out<<"{\"username\":\""<<jsonEscape(rows[i].first)<<"\",\"score\":"<<rows[i].second<<"}";}out<<"]}";return response(out.str());}
 if(method=="GET"&&(path=="/"||path=="/index.html")){ifstream in("index.html",ios::binary);if(!in)return response("index.html not found. Keep it beside server.cpp executable.","text/plain; charset=utf-8",404);ostringstream s;s<<in.rdbuf();return response(s.str(),"text/html; charset=utf-8");}
 return response(err("Route not found."),"application/json; charset=utf-8",404);}
int main(){try{fs::create_directories(DATA_DIR);}catch(const exception&e){cerr<<e.what()<<'\n';return 1;}
#ifdef _WIN32
 WSADATA wsa;if(WSAStartup(MAKEWORD(2,2),&wsa)!=0){cerr<<"Winsock initialization failed\n";return 1;}
#endif
 socket_t server=socket(AF_INET,SOCK_STREAM,0);
#ifdef _WIN32
 if(server==INVALID_SOCKET){cerr<<"Could not create server socket\n";return 1;}
#else
 if(server<0){cerr<<"Could not create server socket\n";return 1;}
#endif
 int yes=1;setsockopt(server,SOL_SOCKET,SO_REUSEADDR,reinterpret_cast<const char*>(&yes),sizeof(yes));sockaddr_in addr{};addr.sin_family=AF_INET;addr.sin_addr.s_addr=htonl(INADDR_ANY);addr.sin_port=htons(PORT);if(bind(server,reinterpret_cast<sockaddr*>(&addr),sizeof(addr))<0||listen(server,16)<0){cerr<<"Could not bind port "<<PORT<<". Is it already in use?\n";CLOSESOCKET(server);return 1;}cout<<"Cyber Detective C++ backend running at http://localhost:"<<PORT<<"\nPress Ctrl+C to stop.\n";
 while(true){sockaddr_in clientAddr{};
#ifdef _WIN32
 int len=sizeof(clientAddr);
#else
 socklen_t len=sizeof(clientAddr);
#endif
 socket_t client=accept(server,reinterpret_cast<sockaddr*>(&clientAddr),&len);
#ifdef _WIN32
 if(client==INVALID_SOCKET)continue;
#else
 if(client<0)continue;
#endif
 string req;char buf[4096];size_t headerEnd=string::npos;size_t contentLength=0;while(req.size()<1024*1024){int n=recv(client,buf,sizeof(buf),0);if(n<=0)break;req.append(buf,n);if(headerEnd==string::npos){headerEnd=req.find("\r\n\r\n");if(headerEnd!=string::npos){string head=req.substr(0,headerEnd);istringstream hs(head);string line;getline(hs,line);while(getline(hs,line)){if(!line.empty()&&line.back()=='\r')line.pop_back();auto colon=line.find(':');if(colon!=string::npos&&lower(trim(line.substr(0,colon)))=="content-length")try{contentLength=stoul(trim(line.substr(colon+1)));}catch(...){}}} }if(headerEnd!=string::npos&&req.size()>=headerEnd+4+contentLength)break;}
 string output;if(headerEnd==string::npos)output=response(err("Malformed request."),"application/json; charset=utf-8",400);else{string head=req.substr(0,headerEnd);istringstream hs(head);string first;getline(hs,first);istringstream fsline(first);string method,target,version;fsline>>method>>target>>version;string body=req.substr(headerEnd+4,contentLength);try{output=handle(method,target,body);}catch(const exception&e){output=response(err(string("Server error: ")+e.what()),"application/json; charset=utf-8",500);}}size_t sent=0;while(sent<output.size()){int n=send(client,output.data()+sent,static_cast<int>(output.size()-sent),0);if(n<=0)break;sent+=static_cast<size_t>(n);}CLOSESOCKET(client);}
 CLOSESOCKET(server);
#ifdef _WIN32
 WSACleanup();
#endif
 return 0;}
