#include<bits/stdc++.h>
using namespace std;
#define debug(x) cout<<"###"<<x<<"###"<<endl;
typedef long long ll;
const double eps=1e-8;
const int INF=0x3f3f3f3f;
const int N=1e3+5;
void printHelp(){   
    //只能处理字母形式。
    printf("请输入一个正规文法G[S]形式,对于空串,用'$' 代替,箭头用'>'代替.\n小写字母表示终结符号，大写字母表示非终结符号.");
    printf("输入中间不要加空格\n严格按照正规式的定义\n");
    printf("example:\nS>aA\nA>a|$\n");
    printf("程序从文件\"intput.txt\"读入，每个规则写一行\n");
}
void init(map<char,int>&CharInt,map<int,char>&IntChar,int (*Graph)[30],int &cnt){
    // 初始化一个终结状态Z
    cnt=0;
    CharInt['Z']=++cnt;
    IntChar[cnt]='Z';
    for(int i=0;i<N;i++){
        memset(Graph[i],0,sizeof Graph[i]);
    }
}
bool AddNode(map<char,int>&CharInt,map<int,char>&IntChar,int(*Graph)[30],string str,int &cnt){
    //  添加节点，建图
    int pos=str.find('>');
    if(pos==-1){
        return false;
    }
    if(CharInt.find(str[0])==CharInt.end()){
        CharInt[str[0]]=++cnt;
        IntChar[cnt]=str[0];
    }
    if(str.length()==3){
        // 输入的是终结    eg:S>a;
        Graph[CharInt[str[0]]][str[2]=='$' ?26:str[0]-'a']=CharInt['Z'];
        return true;
    }
    else if(str.length()==4){
        if(CharInt.find(str[3])==CharInt.end()){ // 新加节点
            CharInt[str[3]]=++cnt;
            IntChar[cnt]=str[3]; // eg:S>aA
        }
        Graph[CharInt[str[0]]][str[3]=='$'?26:str[2]-'a']=CharInt[str[3]];
       return true;
    }
    return false;
}
void PrintGraph(int (*G)[30],int num,map<int,char>IntChar){
   // 根据图之间的关系，打印输出。
   // 输入图关系，有多少个字母,从1开始
   // mp:从int 到char 的映射函数
   cout<<"    ";
   for(int i=0;i<=26;i++){
       if(i==26){
           printf("$");
       }
       else{
           printf("%c ",'a'+i);
        }
   }
   cout<<endl;
   for(int i=1;i<=num;i++){
       cout<<i<<":"<<IntChar[i]<<" ";
       for(int j=0;j<=26;j++){
           if(G[i][j]){
               cout<<G[i][j]<<" ";
           }
           else{
               cout<<"0"<<" ";
           }
       }
       cout<<endl;
   }  
}
set<int> Move(int (*NFA)[30],set<int>st,int a){
    // 输入序号
    // Move 运算,NFA 图
    set<int>res;
    for(auto i:st){
        if(NFA[i][a])res.insert(NFA[i][a]); 
    }
    return res;
}
set<int> EpsClosure(int (*G)[30],set<int> st){
    // 输入图，集合；
    // 输出得到的集合。
    set<int>res=st; 
    for(auto i:st){
        int pos=i;
        while(G[pos][26]){
            res.insert(G[pos][26]);      
            pos=G[pos][26];
        }
    }
    return res;
}
int NFAtoDFA(int (*NFA)[30],int (*DFA)[30],map<char,int>CharInt,map<int,char>IntChar){
    // 将NFA 转化成DFA，返回一个新图中的节点个数,将图传进来作为返回结果.
    // DFA 27维,从0开始  标记 是初态为1, 是终态为2,两者都是为3.
    // 第三个参数：NFACnt的节点个数
    int num=0;// 表示现在的加到多少了
    // eg:G[2][序号]=3 表示 2--25-->3,G的第二维是在字母表中的序号
    map<set<int>,int>vis; // 标记,并且将每个集合映射成一个数
    //vis.clear();
    set<int>S;// init S
    S.insert(CharInt['S']);
    vis[EpsClosure(NFA,S)]=0;    
    while(1){
        bool end=1;
        map<set<int>,int> tmp=vis;
        for(auto i:tmp){
            if(i.second==0){
                end=0;// 表示存在没有被加入的集合
                vis[i.first]=++num;
                for(int j=0;j<26;j++){// 没有空串
                    set<int> U=EpsClosure(NFA,Move(NFA,i.first,j));
                    if(!U.size())continue;
                    if(vis.find(U)!=vis.end()){
                        continue;
                    }
                    vis[U]=0;
                }
            }
        }
        if(end){
            break;
        }
    }
    // 得到集合，开始连边，建图
    for(auto i:vis){
        if(i.first.find(CharInt['S'])!=i.first.end()){
            // 说明找到初始态。
            DFA[i.second][27]+=1;
        }
        if(i.first.find(CharInt['Z'])!=i.first.end()){
            // 找到终态
            DFA[i.second][27]+=2;
        }
        for(int j=0;j<=26;j++){
            set<int> v=EpsClosure(NFA,Move(NFA,i.first,j));
            if(v.size()){
                DFA[i.second][j]=vis[v];
            }
        }
    }
    return num;
}
int MoveGetId(int i,int j,int (*G)[30],map<set<int>,int>mp){
    // i节点走j字母 ，能到的点所属集合的id,如过没有找到，返回-1.
    int dst=G[i][j];
    if(dst==0)return -1;
    for(auto k:mp){
        if(k.first.find(dst)!=k.first.end()){
            return k.second;
        }
        //debug("!");
    }
    return -1;
}
int miniDFA(int (*DFA)[30],int (*res)[30],int num){
    // 将DFA 最小化,输入DFＡ图，返回最小化之后节点数量。
    map<set<int>,int>SetMap;
    set<int>End,NotEnd;
    for(int i=1;i<=num;i++){
        if(DFA[i][27]>=2){
            End.insert(i);
        }
        else{
            NotEnd.insert(i);    
        }
    }
    int cnt=0;
    SetMap[End]=++cnt;SetMap[NotEnd]=++cnt;
    while(1){
        bool flag=1;
        int dst=-1,j;// 表示这个集合中的数所能到的集合下标,j枚举字母
        auto it=SetMap.begin();
        for(;it!=SetMap.end();it++){// 一个集合一个集合拆分
            set<int>u=it->first;
            for(j=0;j<=26;j++){// 枚举字母，尝试划分子集。每次用一个字母划分
                dst=-1;
                for(auto i:u){// 对于u集合中每个的元素i
                    if(!DFA[i][j])continue;
                    int v=MoveGetId(i,j,DFA,SetMap);
                    if(dst==-1){
                        dst=v;
                        continue;
                    }
                    if(v!=dst){
                        // 说明 集合it中内部有分歧。将集合拆分成到达v和不到达v两部分                        
                        flag=0;        
                        break;
                    }
                }
                if(!flag)break;
            }
            if(!flag){// 标志 说明产生分歧，有个集合可以才分了
                break;
            }
        }
        //cout<<"dst: "<<dst<<" setid: "<<it->second<<" setsize: "<<SetMap.size()<<" alpha "<<j<<endl;
        if(!flag){
            int tmpid=it->second;
            set<int>Left,Right;
            //cout<<tmpid<<"frst id "<<endl;
            for(auto i:it->first){// 枚举有分歧中的集合中的元素,进行拆分
               int v=MoveGetId(i,j,DFA,SetMap); 
               if(v==-1)continue;
               if(v==dst){
                    Left.insert(i);
               }
               else{
                   Right.insert(i);
               }
            }
            SetMap.erase(it);
            SetMap[Left]=tmpid;// 分裂，原来的id 替代
            SetMap[Right]=++cnt;
            continue;
        }
        break;
    }
    //  重新连边,建图。并根据集合中的元素标记终态，初态。
    for(auto it:SetMap){//  对于每一个集合
        for(int i:it.first){// 对于集合中的每一个元素
            for(int j=0;j<=26;j++){
                int v=MoveGetId(i,j,DFA,SetMap);
                if(v==-1)continue;
                // 否则 说明 集合中i元素 可以通过字母 j到达v状态
                res[it.second][j]=v;
            }
            res[it.second][27]=max(res[it.second][27],DFA[i][27]);// 继承初态和终态
        }
    }
    return cnt;
}
void PrintMinDFA(int (*G)[30],int cnt){
    cout<<"最后一个数字的含义：1表示初态，2表示终态，3表示两者都是:"<<endl;
    cout<<"   ";
    for(int i=0;i<26;i++){
        printf("%c ",i+'a');
    }
    cout<<"$"<<" ";
    cout<<endl;
    for(int i=1;i<=cnt;i++){
        printf("%c: ",'A'+i-1);
        for(int j=0;j<=27;j++){
            if(G[i][j]&&j<27){
                printf("%c ",'A'+G[i][j]-1);
            }
            else if(j!=27){
                cout<<0<<" ";
            }
            else{
                cout<<G[i][j];
            }
        }
        cout<<endl;
    }
}
int main(){
    string line;
    printHelp();
    map<char,int>CharInt;// 将字符映射到数字,图用数字表示
    map<int,char>IntChar;// 将数字映射到字符
    int Graph[N][30],cnt;// 建图表示一个节点经过某条边，可以达到的另一个节点。
    init(CharInt,IntChar,Graph,cnt);
    ifstream input("./input.txt");
    if(input.is_open()){
        while(getline(input,line)){
            if(!AddNode(CharInt,IntChar,Graph,line,cnt)){// 建图
                printf("Error check input!\n"); 
                break;
            }
        }
    }
    else{
        cout<<"Unable to open file\n"<<endl;
        return 0;
    }
    //PrintGraph(Graph,cnt,IntChar);
    int DFAG[N][30];
    memset(DFAG,0,sizeof DFAG);
    cnt=NFAtoDFA(Graph,DFAG,CharInt,IntChar);// new cnt;
    //  DFAG: 有穷自动机图,每个状态用一个数字表示
 //   for(int i=1;i<=cnt;i++){
 //       cout<<i<<" ";
 //       for(int j=0;j<=27;j++){
 //           cout<<DFAG[i][j]<<" ";
 //       } 
 //       cout<<endl;
 //   }
 //   debug("!");
    int minDFA[N][30];
    memset(minDFA,0,sizeof minDFA);
    cnt=miniDFA(DFAG,minDFA,cnt); 
//    for(int i=1;i<=cnt;i++){
//        cout<<i<<" ";
//        for(int j=0;j<=27;j++){
//            cout<<minDFA[i][j]<<" ";
//        }
//        cout<<endl;
//    }
    PrintMinDFA(minDFA,cnt);// 用于最小化DFA矩阵
}
