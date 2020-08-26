#include <iostream>
#include <string>
#include <cstring>
#include <cassert>
#include <regex>
#include <vector>
#include <map>
using namespace std;
#define MAX_LENGTH 200

enum class Inst
{
    ADD, SUB, MUL, DIV, REM, STORE, LOAD, CE, INVALID
};
enum class Data
{
    MEM, REG, VAL, INVALID
};
struct ASM
{
    Inst inst;
    struct Operand
    {
        int val;
        Data type;
        Operand() : val(0), type(Data::INVALID) {}
        Operand(int t1, Data t2) : val(t1), type(t2) {}
    } op[3];
    ASM() : inst(Inst::INVALID) {}
    ASM(const string &in) : ASM()
    {
        static char t1[30], t2[3][30];
        if(in == "Compile Error!")
            inst = Inst::CE;
        else if(regex_match(in, regex(R"(^(add|sub|mul|div|rem) +r[0-9]+ +(r[0-9]+|[0-9]+) +(r[0-9]+|[0-9]+)$)")))
        {
            sscanf(in.c_str(), "%s%s%s%s", t1, t2[0], t2[1], t2[2]);
            if(!strcmp(t1, "add")) inst = Inst::ADD;
            else if(!strcmp(t1, "sub")) inst = Inst::SUB;
            else if(!strcmp(t1, "mul")) inst = Inst::MUL;
            else if(!strcmp(t1, "div")) inst = Inst::DIV;
            else inst = Inst::REM;
            for(int i=0, tmp; i<3; i++)
            {
                if(t2[i][0] == 'r')
                {
                    sscanf(t2[i], "r%d", &tmp);
                    op[i] = Operand(tmp, Data::REG);
                    if(tmp >= 256 || tmp < 0) inst = Inst::INVALID;
                }
                else
                {
                    sscanf(t2[i], "%d", &tmp);
                    op[i] = Operand(tmp, Data::VAL);
                    if(tmp >= 256 || tmp < 0) inst = Inst::INVALID;
                }
            }
        }
        else if(regex_match(in, regex(R"(^load +r[0-9]+ +\[[0-9]+\]$)")))
        {
            sscanf(in.c_str(), "%*s r%d [%d]", &op[0].val, &op[1].val);
            inst = Inst::LOAD;
            op[0].type = Data::REG;
            op[1].type = Data::MEM;
            if(op[0].val >= 256 || op[0].val < 0) inst = Inst::INVALID;
            if(op[1].val >= 256 || op[1].val < 0) inst = Inst::INVALID;
        }
        else if(regex_match(in, regex(R"(^store +\[[0-9]+\] +r[0-9]+$)")))
        {
            sscanf(in.c_str(), "%*s [%d] r%d", &op[0].val, &op[1].val);
            inst = Inst::STORE;
            op[0].type = Data::MEM;
            op[1].type = Data::REG;
            if(op[0].val >= 256 || op[0].val < 0) inst = Inst::INVALID;
            if(op[1].val >= 256 || op[1].val < 0) inst = Inst::INVALID;
        }
    }
    ASM(const char *in) : ASM(string(in)) {}
};
struct REG
{
    const static int MAX = 256;
    int val[MAX];
    REG() { memset(val, 0, sizeof(val)); }
    int rw(int idx)
    {
        assert(0 <= idx && idx < MAX);
        return val[idx];
    }
    void sw(int idx, int d)
    {
        assert(0 <= idx && idx < MAX);
        val[idx] = d;
    }
    void clear() { memset(val, 0, sizeof(val)); }
};
struct MEM
{
    const static int MAX = 256;
    char *val;
    MEM() { val = new char[MAX]; }
    ~MEM() { delete val; }
    int rw(int idx)
    {
        assert(0 <= idx && idx < MAX);
        int res;
        memcpy(&res, val + idx, sizeof(int));
        return res;
    }
    void sw(int idx, int d)
    {
        assert(0 <= idx && idx < MAX);
        memcpy(val + idx, &d, sizeof(int));
    }
};
vector<ASM> asm_list;

// Return false if the ASM is invalid.
bool insert_ASM(const string &in)
{
    asm_list.emplace_back(ASM(in));
    if(asm_list.back().inst == Inst::INVALID) return false;
    return true;
}

// Return -1 if there exists a "CE" instruction.
tuple<int, int, int> evaluate(const vector<ASM> &list, const vector<int> &xyz = vector<int>())
{
    REG reg;
    MEM mem;
    int val[3];
    for(int i=0; i<(int)xyz.size(); i++)
        mem.sw(i * 4, xyz[i]);
    for(const auto &i : list)
    {
        for(int idx=0; idx<3; idx++)
        {
            switch(i.op[idx].type)
            {
                case Data::REG:
                    val[idx] = reg.rw(i.op[idx].val);
                    break;
                case Data::MEM:
                    val[idx] = mem.rw(i.op[idx].val);
                    break;
                case Data::VAL:
                    val[idx] = i.op[idx].val;
                    break;
                default:
                    break;
            }
        }
        switch(i.inst)
        {
            case Inst::ADD:
                reg.sw(i.op[0].val, val[1] + val[2]);
                break;
            case Inst::SUB:
                reg.sw(i.op[0].val, val[1] - val[2]);
                break;
            case Inst::MUL:
                reg.sw(i.op[0].val, val[1] * val[2]);
                break;
            case Inst::DIV:
                reg.sw(i.op[0].val, val[1] / val[2]);
                break;
            case Inst::REM:
                reg.sw(i.op[0].val, val[1] % val[2]);
                break;
            case Inst::STORE:
                mem.sw(i.op[0].val, val[1]);
                break;
            case Inst::LOAD:
                reg.sw(i.op[0].val, val[1]);
                break;
            case Inst::CE:
                return {mem.rw(0), mem.rw(4), mem.rw(8)};
            default:
                break;
        }
    }
    return {mem.rw(0), mem.rw(4), mem.rw(8)};
}

// Return -1 if there exists a "CE" instruction.
int cycle(const vector<ASM> &list)
{
    const static map<Inst, int> cost = {
        {Inst::ADD, 10}, {Inst::SUB, 10}, 
        {Inst::MUL, 30}, {Inst::DIV, 50}, 
        {Inst::REM, 60}, {Inst::STORE, 200}, 
        {Inst::LOAD, 200}
    };
    int cycle = 0, tmp;
    for(const auto &i : list)
    {
        int penalty = 0;
        switch(i.inst)
        {
            case Inst::ADD:
            case Inst::SUB:
            case Inst::MUL:
            case Inst::DIV:
            case Inst::REM:
            case Inst::STORE:
            case Inst::LOAD:
                tmp = cost.at(i.inst);
                for(const auto &op : i.op)
                    if(op.type == Data::REG && op.val >= 8)
                        penalty = 1;
                break;
            case Inst::CE:
                return -1;
            default:
                break;
        }
        cycle += tmp * (1 + penalty);
    }
    return cycle;
}

// ./ASMC x y z
int main(int argc, char **argv)
{
    vector<int> init;
    if(argc == 4)
        for(int i=1; i<argc; i++)
            init.emplace_back(atoi(argv[i]));
    else init = {2, 3, 5};
    string str;
    int lines = 1;
    while(getline(cin, str))
    {
        if(!insert_ASM(str))
        {
            printf("Instruction invalid at line: %d.\n", lines);
            return 0;
        }
        lines++;
    }
    auto [x, y, z] = evaluate(asm_list, init);
    int C = cycle(asm_list);
    if(C != -1) printf("x, y, z = %d, %d, %d\nTotal cycle = %d\n", x, y, z, C);
    else puts("CE instruction found.");
    return 0;
}