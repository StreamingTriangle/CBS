#include<iostream>
#include<string> 
#define next_s 0
#define next_d 1
#define last_s 2
#define last_d 3

#define out 0
#define in 1
#define bi 2
using namespace std;

string my_to_string(unsigned int a)
{
	string str = "";
	while (a > 0)
	{
		str += (char)('0' + a % 10);
		a = a / 10;
	}
	if (str == "")
		str = "0";
	string astr;
	astr.resize(str.length());
	for (int i = str.length() - 1; i >= 0; i--)
		astr[str.length() - 1 - i] = str[i];
	return astr;
}
class sample_node
{
public:
	unsigned int  nodeID;

	int first_edge[3];

	sample_node* next;

	sample_node()
	{
		nodeID = 0;
		next = NULL;
		for (int i = 0; i < 3; i++)
			first_edge[i] = -1;
	}
	sample_node(unsigned int s, int out_edge_ = -1, int in_edge_ = -1, int bi_edge_ = -1)
	{
		nodeID = s;
		next = NULL;
		first_edge[out] = out_edge_;
		first_edge[in] = in_edge_;
		first_edge[bi] = bi_edge_;
	}
	void init(unsigned int s, int out_edge_ = -1, int in_edge_ = -1, int bi_edge_ = -1)
	{
		nodeID = s;
		next = NULL;
		first_edge[out] = out_edge_;
		first_edge[in] = in_edge_;
		first_edge[bi] = bi_edge_;
	}
	void set_edge(int s, int type)
	{
		first_edge[type] = s;
	}
	int get_edge(int type)
	{
		return first_edge[type];
	}
	void reset()
	{
		nodeID = 0;
		for (int i = 0; i < 3; i++)
			first_edge[i] = -1;
	}
};

class candidate_unit
{
public:
	unsigned int  s, d;
	bool bi_direction;
	double priority;
	long long timestamp;
	int time_list_prev; // suppose the size of the sample table is m. A pointer in value range 0 ~ m-1 means sample unit in the corresponding table pos. 
	//A pointer in value range m ~ 2m-1 means candidate unit in the corresponding table pos. -1 means an empty pointer.
	int time_list_next;
	candidate_unit(unsigned int snum = 0, unsigned int dnum = 0, double p = -1, long long time = -1, int prev = -1, int next = -1, bool bi_direction_ = false)
	{
		s = snum;
		d = dnum;
		priority = p;
		timestamp = time;
		time_list_prev = prev;
		time_list_next = next;
		bi_direction = bi_direction_;
	}
	void reset(unsigned int snum = 0, unsigned int dnum = 0, double p = -1, long long time = -1, int prev = -1, int next = -1, bool bi_direction_ = false)
	{
		s = snum;
		d = dnum;
		priority = p;
		timestamp = time;
		time_list_prev = prev;
		time_list_next = next;
		bi_direction = bi_direction_;
	}
};
class sample_unit
{
public:
	unsigned int s, d;
	bool bi_direction;
	double priority;
	long long timestamp;
	int pointers[4];
	int time_list_prev;
	int time_list_next;
	candidate_unit vice;
	sample_unit(unsigned int snum = 0, unsigned int dnum = 0, double p = -1, long long time = -1, int prev = -1, int next = -1, bool bi_direction_ = false)
	{
		s = snum;
		d = dnum;
		for (int i = 0; i < 4; i++)
			pointers[i] = -1;
		priority = p;
		timestamp = time;
		time_list_prev = prev;
		time_list_next = next;
		bi_direction = bi_direction_;
		vice.reset();
	}
	void set_next_s(int s) { pointers[next_s] = s; }
	void set_next_d(int d) { pointers[next_d] = d; }
	void set_last_s(int s) { pointers[last_s] = s; }
	void set_last_d(int d) { pointers[last_d] = d; }
	void reset(unsigned int snum = 0, unsigned int dnum = 0, double p = -1, long long time = -1, int prev = -1, int next = -1, bool bi_direction_ = false)
	{
		s = snum;
		d = dnum;
		for (int i = 0; i < 4; i++)
			pointers[i] = -1;
		priority = p;
		timestamp = time;
		time_list_prev = prev;
		time_list_next = next;
		bi_direction = bi_direction_;
	}
};

struct edge_info
{
	unsigned int src;
	unsigned int dst;
	long long timestamp;
	double priority;
	bool bidirection;
	edge_info(unsigned int s = 0, unsigned int d = 0, long long time = 0, double p = 0, bool bi_ = false)
	{
		src = s;
		dst = d;
		timestamp = time;
		priority = p;
		bidirection = bi_;
	}
};


