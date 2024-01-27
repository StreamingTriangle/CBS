#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include <math.h> 
#include<algorithm>
#include<unordered_set>
using namespace std;
#define out 0
#define in 1
#define bi 2

int direction_parameters[7][6] = { 
	{out, 0, out, 0, out, 0},
	{out, 1, in, 1, out, 0},
	{bi, 0, in, 0, out, 0},
	{bi, 1, in, 0, in, 0},
	{bi, 1, out, 0, out, 0},
	{out, 0, bi, 0, bi, 0},
	{bi, 1, bi, 1, bi, 1} 
};

struct golden_edge
{
	unsigned int Snode;
	int frequency;
	golden_edge* next;
	
	golden_edge(unsigned int s=0)
	{
		Snode = s;
		next = NULL;
		frequency = 1;
	}
};


struct golden_node
{
	unsigned int ID;
	golden_edge* first_edge[3];

	golden_node(unsigned int s=0)
	{
		ID = s;
		for (int i = 0; i < 3; i++)
			first_edge[i] = NULL;
	}
};

class Graph
{
public:
	unordered_map<unsigned int, golden_node> g;
	int edgecount;
	int nodecount;
	int trianglecount;

	Graph()
	{
		edgecount = 0;
		nodecount = 0;
		trianglecount = 0;
	}
	~Graph()
	{
		unordered_map<unsigned int, golden_node>::iterator it;
		for (it = g.begin(); it != g.end(); it++)
		{
			for (int i = 0; i < 3; i++) {
				golden_edge* tmp = (it->second).first_edge[i];
				golden_edge* next = tmp;
				while (tmp)
				{
					next = tmp->next;
					delete tmp;
					tmp = next;
				}
			}
		}
		g.clear();
	}

	void delete_edge(unsigned int s, unsigned int d, bool bi_direction);

	void insert_edge(unsigned int s, unsigned int d, bool bi_direction);


	int count_triangle(int direction);
	unsigned long long weighted_count(int direction);

	void local_count(unordered_map<unsigned int, int>& cr, int direction);
	void weighted_local(unordered_map<unsigned int, unsigned long long>& cr, int direction);

	void local_count_A(unordered_map<unsigned int, int>& cr);
	void weighted_local_A(unordered_map<unsigned int, unsigned long long> &cr);	

	void local_count_B(unordered_map<unsigned int, int> &cr);
	void weighted_local_B(unordered_map<unsigned int, unsigned long long> &cr);


	void print_graph()
	{
		for (unordered_map<unsigned int, golden_node>::iterator it = g.begin(); it != g.end(); it++)
		{
			cout << "node "<<it->second.ID << endl;
			cout << "out_edge: ";
			golden_edge* tmp = it->second.first_edge[out];
			while (tmp)
			{
				cout << tmp->Snode << ' ';
				tmp = tmp->next;
			}
			cout << endl;

			cout << "in_edge: ";
			tmp = it->second.first_edge[in];
			while (tmp)
			{
				cout << tmp->Snode << ' ';
				tmp = tmp->next;
			}
			cout << endl;

			cout << "bi_edge: ";
			tmp = it->second.first_edge[bi];
			while (tmp)
			{
				cout << tmp->Snode << ' ';
				tmp = tmp->next;
			}
			cout << endl;

		}
	}
	
	int get_edgenum()
	{
		return edgecount;
	}

};

void Graph::insert_edge(unsigned int s, unsigned int d, bool bi_direction) // directed edge s->d, if bool bi_direction is true, then it means a bi_direction edge s<-->d
{

	bool find = false;
	unordered_map<unsigned int, golden_node>::iterator it;
	
	it = g.find(s);
	if (it ==g.end())
	{
		golden_node gn(s);
		if (bi_direction)
			gn.first_edge[bi] = new golden_edge(d);
		else
			gn.first_edge[out] = new golden_edge(d);
		g[s] = gn;
		nodecount++;
	}
	else
	{
		golden_edge* tmp = bi_direction ? (it->second).first_edge[bi] : (it->second).first_edge[out];
		while (tmp)
		{
			if (tmp->Snode == d)
			{
				tmp->frequency++;
				find = true;
				break;
			}
			tmp = tmp->next;
		}
		if (!find)
		{
			golden_edge* tmp = new golden_edge(d);
			if (bi_direction)
			{
				tmp->next = (it->second).first_edge[bi];
				(it->second).first_edge[bi] = tmp;
			}
			else {
				tmp->next = (it->second).first_edge[out];
				(it->second).first_edge[out] = tmp;
			}
		}
	}
	
	
	it = g.find(d);
	if (it ==g.end())
	{
		golden_node gn(d);
		if (bi_direction)
			gn.first_edge[bi] = new golden_edge(s);
		else
			gn.first_edge[in] = new golden_edge(s);
		g[d] = gn;
		nodecount++;
	}
	else
	{
		golden_edge* tmp = bi_direction? (it->second).first_edge[bi]:(it->second).first_edge[in];
		while (tmp)
		{
			if (tmp->Snode == s)
			{
				tmp->frequency++;
				find = true;
				break;
			}
			tmp = tmp->next;
		}
		if (!find)
		{
			golden_edge* tmp = new golden_edge(s);
			if (bi_direction) {
				tmp->next = (it->second).first_edge[bi];
				(it->second).first_edge[bi] = tmp;
			}
			else {
				tmp->next = (it->second).first_edge[in];
				(it->second).first_edge[in] = tmp;
			}
		}
	}
	
	if (!find)
		edgecount++;

	return;
}

void Graph::delete_edge(unsigned int s, unsigned int d, bool bi_direction)
{
	bool deleted = false;
	
	unordered_map<unsigned int, golden_node>::iterator it;

	it = g.find(s);
	if (it == g.end())
		return;
	else
	{
		golden_edge* tmp = bi_direction? (it->second).first_edge[bi]:(it->second).first_edge[out];
		if(!tmp)
			return;
		if (tmp->Snode == d)
		{
			tmp->frequency--;
			if (tmp->frequency == 0)
			{
				if (bi_direction)
					(it->second).first_edge[bi] = tmp->next;
				else
					(it->second).first_edge[out] = tmp->next;
				delete tmp;
				deleted = true;
				if ((it->second).first_edge[out] == NULL && (it->second).first_edge[in] == NULL&&(it->second).first_edge[bi]==NULL)
				{
					g.erase(it);
					nodecount--;
				}
			}
		}
		else
		{
			golden_edge* p = tmp;
			tmp = tmp->next;
			while (tmp)
			{
				if (tmp->Snode == d)
				{
					tmp->frequency--;
					if (tmp->frequency == 0)
					{
						p->next = tmp->next;
						delete tmp;
						deleted = true;
					}
					break;
				}
				p = tmp;
				tmp = tmp->next;
			}
		}
	}
	
	it = g.find(d);
	if (it == g.end())
		return;
	else
	{
		golden_edge* tmp = bi_direction?(it->second).first_edge[bi]:(it->second).first_edge[in];
		if(!tmp)
			return;
		if (tmp->Snode == s)
		{
			tmp->frequency--;
			if (tmp->frequency == 0)
			{
				if (bi_direction)
					(it->second).first_edge[bi] = tmp->next;
				else
					(it->second).first_edge[in] = tmp->next;
				delete tmp;
				deleted = true;
				if ((it->second).first_edge[out] == NULL && (it->second).first_edge[in] == NULL && (it->second).first_edge[bi] == NULL)
				{
					g.erase(it);
					nodecount--;
				}
			}
		}
		else
		{
			golden_edge* p = tmp;
			tmp = tmp->next;
			while (tmp)
			{
				if (tmp->Snode == s)
				{
					tmp->frequency--;
					if (tmp->frequency == 0)
					{
						p->next = tmp->next;
						delete tmp;
						deleted = true;
					}
					break;
				}
				p = tmp;
				tmp = tmp->next;
			}
		}
	}
	
	
	
	if (deleted)
		edgecount--;

	return;

}

int Graph::count_triangle(int direction)
{
	vector<unsigned int> v1;
	unordered_set<unsigned int> neighbor_set;
	golden_edge* cur = NULL;

	int sum = 0;
	unordered_map<unsigned int, golden_node>::iterator it;
	for (it = g.begin(); it != g.end(); it++)
	{
		unsigned int src = it->first;
		cur = (it->second).first_edge[direction_parameters[direction][0]]; // the first parameter determine which list the scan
		while (cur)
		{
			if(direction_parameters[direction][1]==0||cur->Snode<src) // the second parameter determines if we need to filter the list according to ID, in case of counting a triangle multiple times.
				v1.push_back(cur->Snode);
			cur = cur->next;
		}

		cur = (it->second).first_edge[direction_parameters[direction][2]];
		while (cur)
		{
			if (direction_parameters[direction][3] == 0 || cur->Snode < src)
				neighbor_set.insert(cur->Snode);
			cur = cur->next;
		}

		for (int j = 0; j < v1.size(); j++)
		{
			unsigned int anc = v1[j];
			unordered_map<unsigned int, golden_node>::iterator it2 = g.find(anc);
			if (it2 == g.end())
				continue;
			else
				cur = (it2->second).first_edge[direction_parameters[direction][4]];
			while (cur)
			{
				if (direction_parameters[direction][5] == 0 || cur->Snode < anc) {
					if (neighbor_set.find(cur->Snode) != neighbor_set.end()) {
					//	cout << "triangle " << src << " " << anc << ' ' << cur->Snode << endl;
						sum++;
					}
				}
				cur = cur->next;
			}
		}
		v1.clear();
		neighbor_set.clear();
	}
	return sum;
}


unsigned long long Graph::weighted_count(int direction)
{
	vector<pair<unsigned int, int> > v1;
	unordered_map<unsigned int, int> neighbor_freq;
	golden_edge* cur;

	unsigned long long sum = 0;
	unordered_map<unsigned int, golden_node>::iterator it;
	for (it = g.begin(); it != g.end(); it++)
	{
		unsigned int src = it->first;
		//if (src == 3)
			//cout << "break" << endl;
		cur = (it->second).first_edge[direction_parameters[direction][0]];
		while (cur)
		{
			if (direction_parameters[direction][1] == 0 || cur->Snode < src)
			{
				v1.push_back(make_pair(cur->Snode, cur->frequency));
			}
			cur = cur->next;
		}

		cur = (it->second).first_edge[direction_parameters[direction][2]];
		while (cur)
		{
			if (direction_parameters[direction][3] == 0 || cur->Snode < src)
				neighbor_freq[cur->Snode] = cur->frequency;
			cur = cur->next;
		}

		for (int j = 0; j < v1.size(); j++)
		{
			unsigned int anc = v1[j].first;
			int fre1 = v1[j].second;
			unordered_map<unsigned int, golden_node>::iterator it2 = g.find(anc);
			if (it2 == g.end())
				continue;
			else
				cur = (it2->second).first_edge[direction_parameters[direction][4]];
			while (cur)
			{
				if (direction_parameters[direction][5] == 0 || cur->Snode < anc) {
					if (neighbor_freq.find(cur->Snode) != neighbor_freq.end())
					{
						int fre2 = cur->frequency;
						int fre3 = neighbor_freq[cur->Snode];
						sum += ((unsigned long long)fre1 * fre2 * fre3);
					//	cout << "triangle " << src << " " << anc << ' ' << cur->Snode << endl;
						//cout << fre1 << ' ' << fre3 << ' ' << fre2 << endl;
					}
				}
				cur = cur->next;
			}
		}
		v1.clear();
		neighbor_freq.clear();
	}
	return sum;
}

void Graph::local_count(unordered_map<unsigned int, int>& cr, int direction)
{
	vector<unsigned int> v1;
	golden_edge* cur;
	unordered_map<unsigned int, golden_node>::iterator it;
	unordered_set<unsigned int> neighbor_set;

	for (it = g.begin(); it != g.end(); it++)
	{
		unsigned int src = it->first;
		cur = (it->second).first_edge[direction_parameters[direction][0]];
		while (cur)
		{
			if(direction_parameters[direction][1]==0||cur->Snode<src)
				v1.push_back(cur->Snode);
			cur = cur->next;
		}

		cur = (it->second).first_edge[direction_parameters[direction][2]];
		while (cur)
		{
			if (direction_parameters[direction][3] == 0 || cur->Snode < src)
				neighbor_set.insert(cur->Snode);
			cur = cur->next;
		}

		int total_count = 0;

		for (int j = 0; j < v1.size(); j++)
		{
			int sum = 0;
			unsigned int anc = v1[j];
			unordered_map<unsigned int, golden_node>::iterator it2 = g.find(anc);
			if (it2 == g.end())
				continue;
			else
				cur = (it2->second).first_edge[direction_parameters[direction][4]];
			while (cur)
			{
				if (direction_parameters[direction][5] == 0 || cur->Snode < anc) {
					if (neighbor_set.find(cur->Snode) != neighbor_set.end())
					{
						sum++;
						if (cr.find(cur->Snode) != cr.end())
							cr[cur->Snode]++;
						else
							cr[cur->Snode] = 1;
					}
				}
				cur = cur->next;
			}
			if (cr.find(anc) != cr.end())
				cr[anc] += sum;
			else
				cr[anc] = sum;
			total_count += sum;
		}
		v1.clear();
		neighbor_set.clear();
		if (cr.find(src) != cr.end())
			cr[src] += total_count;
		else
			cr[src] = total_count;
	}

	for (it = g.begin(); it != g.end(); it++)
	{
		if (cr.find(it->first) == cr.end())
			cr[it->first] = 0;
	}
}


void Graph::weighted_local(unordered_map<unsigned int, unsigned long long>& cr, int direction)
{
	vector<pair<unsigned int, int> > v1;
	golden_edge* cur;
	unordered_map<unsigned int, golden_node>::iterator it;
	unordered_map<unsigned int, int> neighbor_freq;

	for (it = g.begin(); it != g.end(); it++)
	{
		unsigned int src = it->first;
		cur = (it->second).first_edge[direction_parameters[direction][0]];
		while (cur)
		{
			if (direction_parameters[direction][1] == 0 || cur->Snode < src)
				v1.push_back(make_pair(cur->Snode, cur->frequency));
			cur = cur->next;
		}

		cur = (it->second).first_edge[direction_parameters[direction][2]];
		while (cur)
		{
			if (direction_parameters[direction][3] == 0 || cur->Snode < src)
				neighbor_freq[cur->Snode] = cur->frequency;
			cur = cur->next;
		}
		unsigned long long total_count = 0;

		for (int j = 0; j < v1.size(); j++)
		{
			unsigned long long sum = 0;
			unsigned int anc = v1[j].first;
			int fre1 = v1[j].second;
			unordered_map<unsigned int, golden_node>::iterator it2 = g.find(anc);
			if (it2 == g.end())
				continue;
			else
				cur = (it2->second).first_edge[direction_parameters[direction][4]];
			while (cur)
			{
				if (direction_parameters[direction][5] == 0 || cur->Snode < anc) {
					if (neighbor_freq.find(cur->Snode) != neighbor_freq.end())
					{
						int fre2 = cur->frequency;
						int fre3 = neighbor_freq[cur->Snode];
						unsigned long long triangle_fre = ((unsigned long long)fre1 * fre2 * fre3);

						sum += triangle_fre;

						if (cr.find(cur->Snode) != cr.end())
							cr[cur->Snode] += triangle_fre;
						else
							cr[cur->Snode] = triangle_fre;
					}
				}
				cur = cur->next;
			}
			if (cr.find(anc) != cr.end())
				cr[anc] += sum;
			else
				cr[anc] = sum;
			total_count += sum;
		}
		v1.clear();
		neighbor_freq.clear();
		if (cr.find(src) != cr.end())
			cr[src] += total_count;
		else
			cr[src] = total_count;
	}

	for (it = g.begin(); it != g.end(); it++)
	{
		if (cr.find(it->first) == cr.end())
			cr[it->first] = 0;
	}
	return;
}




void Graph::local_count_A(unordered_map<unsigned int, int> &cr)
{
	vector<unsigned int> v1;
	golden_edge* cur;
	unordered_map<unsigned int, golden_node>::iterator it;
	unordered_set<unsigned int> neighbor_set;
	
	for (it = g.begin(); it != g.end(); it++)
	{
		unsigned int src = it->first;
		cur = (it->second).first_edge[out];
		while (cur)
		{
			v1.push_back(cur->Snode);
			neighbor_set.insert(cur->Snode);
			cur = cur->next;
		}
		int total_count = 0;
		
		for (int j = 0; j < v1.size(); j++)
		{
			int sum = 0;
			unsigned int anc = v1[j];
			unordered_map<unsigned int, golden_node>::iterator it2 = g.find(anc);
			if (it2 == g.end())
				continue;
			else
				cur = (it2->second).first_edge[out];
			while (cur)
			{
				if(neighbor_set.find(cur->Snode)!=neighbor_set.end())
				{
					sum++;
					if(cr.find(cur->Snode)!=cr.end())
						cr[cur->Snode]++;
					else
						cr[cur->Snode] = 1;
				}
				cur = cur->next;
			}
			if(cr.find(anc)!=cr.end())
				cr[anc]+= sum;
			else
				cr[anc] = sum;	
			total_count += sum;
		}
		v1.clear();
		neighbor_set.clear();
		if(cr.find(src)!=cr.end())
			cr[src] += total_count;
		else
			cr[src] = total_count;
	}
	
	for (it = g.begin(); it != g.end(); it++)
	{
		if(cr.find(it->first)==cr.end())
			cr[it->first] = 0;
	} 
}

void Graph::weighted_local_A(unordered_map<unsigned int, unsigned long long> &cr)
{
	vector<unsigned int> v1;
	golden_edge* cur;
	unordered_map<unsigned int, golden_node>::iterator it;
	unordered_map<unsigned int, int> neighbor_freq;
	
	for (it = g.begin(); it != g.end(); it++)
	{
		unsigned int src = it->first;
		cur = (it->second).first_edge[out];
		while (cur)
		{
			v1.push_back(cur->Snode);
			neighbor_freq[cur->Snode] = cur->frequency;
			cur = cur->next;
		}
		unsigned long long total_count = 0;
		
		for (int j = 0; j < v1.size(); j++)
		{
			unsigned long long sum = 0;
			unsigned int anc = v1[j];
			int fre1 = neighbor_freq[anc];
			unordered_map<unsigned int, golden_node>::iterator it2 = g.find(anc);
			if (it2 == g.end())
				continue;
			else
				cur = (it2->second).first_edge[out];
			while (cur)
			{
				if(neighbor_freq.find(cur->Snode)!=neighbor_freq.end())
				{
					int fre2 = cur->frequency;
					int fre3 = neighbor_freq[cur->Snode];
					unsigned long long triangle_fre = ((unsigned long long)fre1*fre2*fre3);
					
					sum += triangle_fre;
					
					if(cr.find(cur->Snode)!=cr.end())
						cr[cur->Snode] += triangle_fre;
					else
						cr[cur->Snode] = triangle_fre;
				}
				cur = cur->next;
			}
			if(cr.find(anc)!=cr.end())
				cr[anc]+= sum;
			else
				cr[anc] = sum;	
			total_count += sum;
		}
		v1.clear();
		neighbor_freq.clear();
		if(cr.find(src)!=cr.end())
			cr[src] += total_count;
		else
			cr[src] = total_count;
	}
	
	for (it = g.begin(); it != g.end(); it++)
	{
		if(cr.find(it->first)==cr.end())
			cr[it->first] = 0;
	} 
	return;
}

void Graph::local_count_B(unordered_map<unsigned int, int> &cr)
{
	vector<unsigned int> v1;
	golden_edge* cur;
	unordered_map<unsigned int, golden_node>::iterator it;
	unordered_set<unsigned int> neighbor_set;
	
	for (it = g.begin(); it != g.end(); it++)
	{
		unsigned int src = it->first;
		
		cur = (it->second).first_edge[out];
		while (cur)
		{
			if(cur->Snode<src) 
				v1.push_back(cur->Snode);
			cur = cur->next;
		}
		
		cur = (it->second).first_edge[in];
		while (cur)
		{
			if(cur->Snode<src)
				neighbor_set.insert(cur->Snode);
			cur = cur->next;
		}
		
		int total_count = 0;
		
		for (int j = 0; j < v1.size(); j++)
		{
			int sum = 0;
			unsigned int anc = v1[j];
			unordered_map<unsigned int, golden_node>::iterator it2 = g.find(anc);
			if (it2 == g.end())
				continue;
			else
				cur = (it2->second).first_edge[out];
			while (cur)
			{
				if(neighbor_set.find(cur->Snode)!=neighbor_set.end())
				{
					sum++;
					if(cr.find(cur->Snode)!=cr.end())
						cr[cur->Snode]++;
					else
						cr[cur->Snode] = 1;
				}
				cur = cur->next;
			}
			if(cr.find(anc)!=cr.end())
				cr[anc]+= sum;
			else
				cr[anc] = sum;	
			total_count += sum;
		}
		v1.clear();
		neighbor_set.clear();
		if(cr.find(src)!=cr.end())
			cr[src] += total_count;
		else
			cr[src] = total_count;
	}
	
	for (it = g.begin(); it != g.end(); it++)
	{
		if(cr.find(it->first)==cr.end())
			cr[it->first] = 0;
	} 
}

void Graph::weighted_local_B(unordered_map<unsigned int, unsigned long long> &cr)
{
	vector<unsigned int> v1;
	golden_edge* cur;
	unordered_map<unsigned int, golden_node>::iterator it;
	unordered_map<unsigned int, int> out_neighbor_freq;
	unordered_map<unsigned int, int> in_neighbor_freq;
	
	for (it = g.begin(); it != g.end(); it++)
	{
		unsigned int src = it->first;
		
		cur = (it->second).first_edge[out];
		while (cur)
		{
			if(cur->Snode<src) 
			{
				v1.push_back(cur->Snode);
				out_neighbor_freq[cur->Snode] = cur->frequency;
			}
			cur = cur->next;
		}
		
		cur = (it->second).first_edge[in];
		while (cur)
		{
			if(cur->Snode<src)
				in_neighbor_freq[cur->Snode] = cur->frequency;
			cur = cur->next;
		}
		
		unsigned long long total_count = 0;
		
		for (int j = 0; j < v1.size(); j++)
		{
			unsigned long long sum = 0;
			unsigned int anc = v1[j];
			int fre1 = out_neighbor_freq[anc];
			
			unordered_map<unsigned int, golden_node>::iterator it2 = g.find(anc);
			if (it2 == g.end())
				continue;
			else
				cur = (it2->second).first_edge[out];
			while (cur)
			{
				if(in_neighbor_freq.find(cur->Snode)!=in_neighbor_freq.end())
				{
					int fre2 = cur->frequency;
					int fre3 = in_neighbor_freq[cur->Snode];
					unsigned long long triangle_fre = ((unsigned long long)fre1*fre2*fre3);
					
					sum += triangle_fre;
					if(cr.find(cur->Snode)!=cr.end())
						cr[cur->Snode]+= triangle_fre;
					else
						cr[cur->Snode] = triangle_fre;;
				}
				cur = cur->next;
			}
			if(cr.find(anc)!=cr.end())
				cr[anc]+= sum;
			else
				cr[anc] = sum;	
			total_count += sum;
		}
		v1.clear();
		out_neighbor_freq.clear();
		in_neighbor_freq.clear();
		if(cr.find(src)!=cr.end())
			cr[src] += total_count;
		else
			cr[src] = total_count;
	}
	
	for (it = g.begin(); it != g.end(); it++)
	{
		if(cr.find(it->first)==cr.end())
			cr[it->first] = 0;
	} 
}
