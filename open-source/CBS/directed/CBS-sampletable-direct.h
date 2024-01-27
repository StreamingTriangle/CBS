#include<iostream>
#include<vector>
#include<assert.h>
#include<math.h>
#include<unordered_map>
#ifndef setting
#include "../../Common/directed/directed-setting.h"
#define setting
#endif

using namespace std;

class CBS_sampletable
{
public:
	EdgeTable* edge_table;
	NodeTable* node_table;
	int group_num;// edge table is divided into segments of group, each group has a q_count and a valid_num;
	int total_size; // total_size of the edge table;
	int group_size; // size of each group. total_size / group_num;
	int* edge_count;
	int node_count;
	int* valid_num;
	double* q_count;
	long long current_time;
	long long* land_mark;
	long long* last_mark;
	int gswitch_iter; // use a iterator to mark the group with largest group ID and has switched slice. This iterator increase from 0 to group_num and return to 0;
	int window_size;

	CBS_sampletable(int size, int window_size_, int group)
	{
		total_size = size;
		group_num = group;
		group_size = total_size/group_num;
		node_count = 0;
		edge_table = new EdgeTable(size);
		node_table = new NodeTable(4, 2*size);
		
		valid_num = new int[group_num];
		q_count = new double[group_num];
		edge_count = new int[group_num];
		window_size = window_size_;

		for(int i=0;i<group_num;i++)
		{
			valid_num[i] = 0;
			q_count[i] = group_size;
			edge_count[i] = 0;
		}
		land_mark = new long long[group_num];
		last_mark = new long long[group_num];
		for (int i = 0; i < group_num; i++)
		{
			land_mark[i] = -(group_num - i) * (window_size / group_num);
		}
		gswitch_iter = 0;

	}
	~CBS_sampletable()
	{
		delete edge_table;
		delete node_table;
		delete []q_count;
		delete []valid_num;
		delete []edge_count;
	}


	void get_neighbor_list(unsigned int v, int direction, vector < pair<unsigned int, long long>>& neighbor_vec)
	{
		sample_node* pos_v = node_table->ID_to_pos(v);
		if (!pos_v)
			return;
		int edge_v = pos_v->first_edge[direction];
		while (edge_v >= 0)
		{
			unsigned int tmp = (edge_table->table[edge_v].s == v ? edge_table->table[edge_v].d : edge_table->table[edge_v].s);
			if (edge_table->table[edge_v].vice.timestamp<0 || edge_table->table[edge_v].vice.timestamp>edge_table->table[edge_v].timestamp)
				neighbor_vec.push_back(make_pair(tmp, edge_table->table[edge_v].timestamp));
			edge_v = (edge_table->table[edge_v].s == v ? edge_table->table[edge_v].pointers[last_s] : edge_table->table[edge_v].pointers[last_d]);
		}
	}

	void get_neighbor_list(unsigned int v, int direction, unordered_map<unsigned int, vector<long long>>& neighbor_vec)
	{
		sample_node* pos_v = node_table->ID_to_pos(v);
		if (!pos_v)
			return;
		int edge_v = pos_v->first_edge[direction];
		while (edge_v >= 0)
		{
			unsigned int tmp = (edge_table->table[edge_v].s == v ? edge_table->table[edge_v].d : edge_table->table[edge_v].s);
			if (edge_table->table[edge_v].vice.timestamp<0 || edge_table->table[edge_v].vice.timestamp>edge_table->table[edge_v].timestamp)
				neighbor_vec[tmp].push_back(edge_table->table[edge_v].timestamp);
			edge_v = (edge_table->table[edge_v].s == v ? edge_table->table[edge_v].pointers[last_s] : edge_table->table[edge_v].pointers[last_d]);
		}
	}

	void link_list(sample_node* pos_s, sample_node* pos_d, bool bi_direction, int pos)
	{
		unsigned int s_num = pos_s->nodeID;
		unsigned int d_num = pos_d->nodeID;

		if (!bi_direction) {
			edge_table->table[pos].set_last_s(pos_s->first_edge[out]);
			edge_table->table[pos].set_last_d(pos_d->first_edge[in]);

			if (pos_s->first_edge[out] >= 0)
				edge_table->table[pos_s->first_edge[out]].set_next_s(pos);

			if (pos_d->first_edge[in] >= 0)
				edge_table->table[pos_d->first_edge[in]].set_next_d(pos);
			pos_s->set_edge(pos, out);
			pos_d->set_edge(pos, in);			// set the cross list;
		}
		else
		{
			edge_table->table[pos].set_last_s(pos_s->first_edge[bi]);
			edge_table->table[pos].set_last_d(pos_d->first_edge[bi]);

			if (pos_s->first_edge[bi] >= 0)
			{
				if (edge_table->table[pos_s->first_edge[bi]].s == s_num)
					edge_table->table[pos_s->first_edge[bi]].set_next_s(pos);
				else
					edge_table->table[pos_s->first_edge[bi]].set_next_d(pos);
			}

			if (pos_d->first_edge[bi] >= 0)
			{
				if (edge_table->table[pos_d->first_edge[bi]].s == d_num)
					edge_table->table[pos_d->first_edge[bi]].set_next_s(pos);
				else
					edge_table->table[pos_d->first_edge[bi]].set_next_d(pos);
			}
			pos_s->set_edge(pos, bi);
			pos_d->set_edge(pos, bi);
		}

	}
	void dismiss(unsigned int old_s, unsigned int old_d, bool bi_direction, int pos)
	{
		int last_edge_s = edge_table->table[pos].pointers[last_s];  // isolate this edge from the list
		int last_edge_d = edge_table->table[pos].pointers[last_d];
		int next_edge_s = edge_table->table[pos].pointers[next_s];
		int next_edge_d = edge_table->table[pos].pointers[next_d];


		if (!bi_direction) {

			sample_node* pos_s = node_table->ID_to_pos(old_s);
			if (pos_s->first_edge[out] == pos)
			{
				if (last_edge_s < 0 && pos_s->first_edge[in] < 0 && pos_s->first_edge[bi] < 0) // there are no edges left for this node
				{
					node_table->delete_via_ID(old_s);
					node_count--;
				}
				else
					pos_s->first_edge[out] = last_edge_s;
			}

			sample_node* pos_d = node_table->ID_to_pos(old_d);

			if (pos_d->first_edge[in] == pos)
			{
				if (last_edge_d < 0 && pos_d->first_edge[out] < 0 && pos_d->first_edge[bi] < 0)
				{
					node_table->delete_via_ID(pos_d->nodeID);
					node_count--;
				}
				else
					pos_d->first_edge[in] = last_edge_d;
			}


			if (last_edge_s >= 0)
				edge_table->table[last_edge_s].set_next_s(next_edge_s);

			if (next_edge_s >= 0)
				edge_table->table[next_edge_s].set_last_s(last_edge_s);

			if (last_edge_d >= 0)
				edge_table->table[last_edge_d].set_next_d(next_edge_d);

			if (next_edge_d >= 0)
				edge_table->table[next_edge_d].set_last_d(last_edge_d);
		}
		else
		{
			sample_node* pos_s = node_table->ID_to_pos(old_s);
			if (pos_s->first_edge[bi] == pos)
			{
				if (last_edge_s < 0 && pos_s->first_edge[in] < 0 && pos_s->first_edge[out] < 0) // there are no edges left for this node
				{
					node_table->delete_via_ID(pos_s->nodeID);
					node_count--;
				}
				else
					pos_s->first_edge[bi] = last_edge_s;
			}
			sample_node* pos_d = node_table->ID_to_pos(old_d);

			if (pos_d->first_edge[bi] == pos)
			{
				if (last_edge_d < 0 && pos_d->first_edge[in] < 0 && pos_d->first_edge[out] < 0)
				{
					node_table->delete_via_ID(pos_d->nodeID);
					node_count--;
				}
				else
					pos_d->first_edge[bi] = last_edge_d;
			}


			if (last_edge_s >= 0)
			{
				if (edge_table->table[last_edge_s].s == old_s)
					edge_table->table[last_edge_s].set_next_s(next_edge_s);
				else
					edge_table->table[last_edge_s].set_next_d(next_edge_s);
			}

			if (next_edge_s >= 0)
			{
				if (edge_table->table[next_edge_s].s == old_s)
					edge_table->table[next_edge_s].set_last_s(last_edge_s);
				else
					edge_table->table[next_edge_s].set_last_d(last_edge_s);
			}

			if (last_edge_d >= 0)
			{
				if (edge_table->table[last_edge_d].d == old_d)
					edge_table->table[last_edge_d].set_next_d(next_edge_d);
				else
					edge_table->table[last_edge_d].set_next_s(next_edge_d);
			}

			if (next_edge_d >= 0)
			{
				if (edge_table->table[next_edge_d].d == old_d)
					edge_table->table[next_edge_d].set_last_d(last_edge_d);
				else
					edge_table->table[next_edge_d].set_last_s(last_edge_d);
			}
		}

		return;
	}

	void dismiss(sample_node* pos_s, sample_node* pos_d, bool bi_direction, int pos)
	{

		unsigned int old_s = pos_s->nodeID;
		unsigned int old_d = pos_d->nodeID;
		int last_edge_s = edge_table->table[pos].pointers[last_s];  // isolate this edge from the list
		int last_edge_d = edge_table->table[pos].pointers[last_d];
		int next_edge_s = edge_table->table[pos].pointers[next_s];
		int next_edge_d = edge_table->table[pos].pointers[next_d];


		if (!bi_direction) {
			if (pos_s->first_edge[out] == pos)
			{
				if (last_edge_s < 0 && pos_s->first_edge[in] < 0 && pos_s->first_edge[bi] < 0) // there are no edges left for this node
				{
					node_table->delete_via_ID(old_s);
					node_count--;
				}
				else
					pos_s->first_edge[out] = last_edge_s;
			}

			if (!pos_d || pos_d->nodeID != old_d)
				pos_d = node_table->ID_to_pos(old_d);	// after delete old_s, the address of old_d in the nodetable may have changed

			if (pos_d->first_edge[in] == pos)
			{
				if (last_edge_d < 0 && pos_d->first_edge[out] < 0 && pos_d->first_edge[bi] < 0)
				{
					node_table->delete_via_ID(pos_d->nodeID);
					node_count--;
				}
				else
					pos_d->first_edge[in] = last_edge_d;
			}


			if (last_edge_s >= 0)
				edge_table->table[last_edge_s].set_next_s(next_edge_s);

			if (next_edge_s >= 0)
				edge_table->table[next_edge_s].set_last_s(last_edge_s);

			if (last_edge_d >= 0)
				edge_table->table[last_edge_d].set_next_d(next_edge_d);

			if (next_edge_d >= 0)
				edge_table->table[next_edge_d].set_last_d(last_edge_d);
		}
		else
		{
			if (pos_s->first_edge[bi] == pos)
			{
				if (last_edge_s < 0 && pos_s->first_edge[in] < 0 && pos_s->first_edge[out] < 0) // there are no edges left for this node
				{
					node_table->delete_via_ID(pos_s->nodeID);
					node_count--;
				}
				else
					pos_s->first_edge[bi] = last_edge_s;
			}
			if (!pos_d || pos_d->nodeID != old_d)
				pos_d = node_table->ID_to_pos(old_d);

			if (pos_d->first_edge[bi] == pos)
			{
				if (last_edge_d < 0 && pos_d->first_edge[in] < 0 && pos_d->first_edge[out] < 0)
				{
					node_table->delete_via_ID(pos_d->nodeID);
					node_count--;
				}
				else
					pos_d->first_edge[bi] = last_edge_d;
			}


			if (last_edge_s >= 0)
			{
				if (edge_table->table[last_edge_s].s == old_s)
					edge_table->table[last_edge_s].set_next_s(next_edge_s);
				else
					edge_table->table[last_edge_s].set_next_d(next_edge_s);
			}

			if (next_edge_s >= 0)
			{
				if (edge_table->table[next_edge_s].s == old_s)
					edge_table->table[next_edge_s].set_last_s(last_edge_s);
				else
					edge_table->table[next_edge_s].set_last_d(last_edge_s);
			}

			if (last_edge_d >= 0)
			{
				if (edge_table->table[last_edge_d].d == old_d)
					edge_table->table[last_edge_d].set_next_d(next_edge_d);
				else
					edge_table->table[last_edge_d].set_next_s(next_edge_d);
			}

			if (next_edge_d >= 0)
			{
				if (edge_table->table[next_edge_d].d == old_d)
					edge_table->table[next_edge_d].set_last_d(last_edge_d);
				else
					edge_table->table[next_edge_d].set_last_s(last_edge_d);
			}
		}

		return;
	}


void insert(unsigned int s_num, unsigned int d_num, long long time, bool bi_direction)
	{
		double p = (rand() % 10000 + 1) / double(10000 + 2);
		int g = rand() % group_num;
		current_time = time;
		group_switch(time);
		long long local_landmark = land_mark[g];
		long long local_lastmark = last_mark[g];
		unsigned int pos = (rand() % group_size) + group_size * g;




		if (edge_table->table[pos].s == 0 && edge_table->table[pos].d == 0)
		{
			sample_node* pos_s = node_table->ID_to_pos(s_num);
			sample_node* pos_d = node_table->ID_to_pos(d_num);
			if (!pos_s)
			{
				pos_s = node_table->insert(s_num);
				node_count++;
			}
			if (!pos_d)
			{
				pos_d = node_table->insert(d_num);
				node_count++;
			}					// if the node is not in the table ,insert it



			if (edge_table->table[pos].vice.timestamp >= 0)// there may be this case: the substream has not received a new item for a int time, and the old sample become test item, but no sample;
			{
				assert(edge_table->table[pos].vice.timestamp < local_landmark && edge_table->table[pos].vice.timestamp >= local_lastmark);

				if (p >= edge_table->table[pos].vice.priority)
				{
					valid_num[g]++;
					edge_table->replace_sample(s_num, d_num, p, time, bi_direction,pos);
					link_list(pos_s, pos_d, bi_direction,pos);
					q_count[g] = q_count[g] - 1 / pow(2, int(-(log(1 - edge_table->table[pos].vice.priority) / log(2))) + 1) + 1 / pow(2, int(-(log(1 - p) / log(2))) + 1);
					edge_table->delete_vice(pos);
				}
				else
				{
					edge_table->replace_sample(s_num, d_num, p, time, bi_direction, pos);
					link_list(pos_s, pos_d, bi_direction, pos); // remain invalid
				}

			}
			else{
				edge_count[g]++;
				valid_num[g]++;
				edge_table->replace_sample(s_num, d_num, p, time, bi_direction, pos);	// if there is no sampled edge in this substream, this is the first one
			//	cout << pos << endl;
				link_list(pos_s, pos_d, bi_direction, pos);
				q_count[g] = q_count[g] - 1 + 1 / pow(2, int(-(log(1 - p) / log(2))) + 1);
			}
			return;
		}

		// else if the sampled edge is in last slice

		if (edge_table->table[pos].timestamp<local_landmark)
		{
			if (p >= edge_table->table[pos].priority)// if larger than the sampled p, replace it;
			{
				if (!(edge_table->table[pos].vice.timestamp >= local_landmark || edge_table->table[pos].vice.timestamp < 0))
				{
					cout << "error " << pos<<' '<<edge_table->table[pos].vice.timestamp << ' ' << land_mark << endl;
					unsigned int check_pos1 = edge_table->tsl_head;
					check_pos1 = check_pos1 % total_size;

					unsigned int check_pos2 = 6968;
					cout << edge_table->table[check_pos1].timestamp <<' '<<edge_table->table[check_pos2].timestamp<<endl;
				}
				// in this case, the vice edge is not needed any way;
				edge_table->delete_vice(pos);
				// reset the vice edge

				// replace the sample edge
				sample_node* old_s = node_table->ID_to_pos(edge_table->table[pos].s);
				sample_node* old_d = node_table->ID_to_pos(edge_table->table[pos].d);
				bool old_bi = edge_table->table[pos].bi_direction;
				double var = -1 / pow(2, int(-(log(1 - edge_table->table[pos].priority) / log(2))) + 1) + 1 / pow(2, int(-(log(1 - p) / log(2))) + 1);
				q_count[g] += var;
				dismiss(old_s, old_d, old_bi, pos);
				edge_table->replace_sample(s_num, d_num, p, time, bi_direction, pos);


				sample_node* pos_s = node_table->ID_to_pos(s_num);
				sample_node* pos_d = node_table->ID_to_pos(d_num);
				if (!pos_s)
				{
					pos_s = node_table->insert(s_num);
					node_count++;
				}
				if (!pos_d)
				{
					pos_d = node_table->insert(d_num);
					node_count++;
				}					// if the node is not in the table ,insert it
				link_list(pos_s, pos_d, bi_direction,pos);
			}
			else   // if smaller than the sampled p, check the vice edge;
			{
				if (p >= edge_table->table[pos].vice.priority) // can replace it;
					edge_table->replace_vice(s_num, d_num, p, time, bi_direction,pos);
			}
		}
		else  //the sample edge is larger than the landmark
		{

			if (p >= edge_table->table[pos].priority)// if larger than the sampled p, replace it;
			{
				// in this case, we need to check the vice edge,

				if (edge_table->table[pos].vice.timestamp < local_landmark &&edge_table->table[pos].vice.timestamp >= 0) // then this is a test edge
				{

					assert(edge_table->table[pos].vice.timestamp >= local_lastmark);
					if (p >= edge_table->table[pos].vice.priority) // the new edge can replace the test edge
					{
						q_count[g] = q_count[g] - 1 / pow(2, int(-(log(1 - edge_table->table[pos].vice.priority) / log(2))) + 1) + 1 / pow(2, int(-(log(1 - p) / log(2))) + 1);

						edge_table->delete_vice(pos); // the test edge is not needed any more;

						dismiss(edge_table->table[pos].s, edge_table->table[pos].d, edge_table->table[pos].bi_direction, pos);
						edge_table->replace_sample(s_num, d_num, p, time, bi_direction, pos);   // the triangle count need not to be deleted, as a test edge means the sample is not valid.

						sample_node* pos_s = node_table->ID_to_pos(s_num);
						sample_node* pos_d = node_table->ID_to_pos(d_num);
						if (!pos_s)
						{
							pos_s = node_table->insert(s_num);
							node_count++;
						}
						if (!pos_d)
						{
							pos_d = node_table->insert(d_num);
							node_count++;
						}					// if the node is not in the table ,insert it

						link_list(pos_s, pos_d, bi_direction, pos);
						valid_num[g]++;
					}
					else		// sample still invalid.
					{
						dismiss(edge_table->table[pos].s, edge_table->table[pos].d, edge_table->table[pos].bi_direction, pos);
						edge_table->replace_sample(s_num, d_num, p, time, bi_direction, pos);

						sample_node* pos_s = node_table->ID_to_pos(s_num);
						sample_node* pos_d = node_table->ID_to_pos(d_num);
						if (!pos_s)
						{
							pos_s = node_table->insert(s_num);
							node_count++;
						}
						if (!pos_d)
						{
							pos_d = node_table->insert(d_num);
							node_count++;
						}					// if the node is not in the table ,insert it
						link_list(pos_s, pos_d, bi_direction,pos);
					}
				}
				else //else there should be no vice edge, we replace the sampled edge.
				{
					sample_node* old_s = node_table->ID_to_pos(edge_table->table[pos].s);
					sample_node* old_d = node_table->ID_to_pos(edge_table->table[pos].d);
					bool old_bi = edge_table->table[pos].bi_direction;

					q_count[g] = q_count[g] - 1 / pow(2, int(-(log(1 - edge_table->table[pos].priority) / log(2))) + 1) + 1 / pow(2, int(-(log(1 - p) / log(2))) + 1);

						
					dismiss(old_s, old_d, old_bi, pos);
					edge_table->replace_sample(s_num, d_num, p, time, bi_direction, pos);

					sample_node* pos_s = node_table->ID_to_pos(s_num);
					sample_node* pos_d = node_table->ID_to_pos(d_num);
					if (!pos_s)
					{
						pos_s = node_table->insert(s_num);
						node_count++;
					}
					if (!pos_d)
					{
						pos_d = node_table->insert(d_num);
						node_count++;
					}					// if the node is not in the table ,insert it
					link_list(pos_s, pos_d, bi_direction, pos);

				}

			}

		}

		return;
	}


	edge_info expire_one_edge(long long time) 
	{
		int tsl_pos = edge_table->tsl_head;
		if(tsl_pos < 0)
			return edge_info(0,0,0,0);
		int pos = tsl_pos % total_size;
		
			
		if (edge_table->table[pos].timestamp < time) // expired edges will only appear as sampled edges. 
		{
		
			int g = pos / group_size;
			tsl_pos = edge_table->table[pos].time_list_next;

			if (edge_table->table[pos].vice.timestamp >= time)
			{
				sample_node* old_s = node_table->ID_to_pos(edge_table->table[pos].s);
				sample_node* old_d = node_table->ID_to_pos(edge_table->table[pos].d);
				bool old_bi = edge_table->table[pos].bi_direction;
				edge_info return_result(edge_table->table[pos].s, edge_table->table[pos].d, edge_table->table[pos].timestamp, edge_table->table[pos].priority, old_bi);
			//	modify_triangle(old_s, old_d, -1);
				dismiss(old_s, old_d, old_bi,pos);
				sample_unit tmp = edge_table->table[pos];

				edge_table->delete_sample(pos); // delete the expired sample;


				valid_num[g]--;  // the valid num decreases;


				edge_table->table[pos].reset(tmp.vice.s, tmp.vice.d, tmp.vice.priority, tmp.vice.timestamp, tmp.vice.time_list_prev, tmp.vice.time_list_next, tmp.vice.bi_direction); // the vice edge is an invalid sample now
				
			//	assert(edge_table->table[tmp.vice.time_list_prev%total_size].time_list_next == pos + total_size);
	
				edge_table->set_time_list(tmp.vice.time_list_prev, 1, pos);
				edge_table->set_time_list(tmp.vice.time_list_next, 0, pos); // the pointer used to be pos + total_size (candidate unit), now updated to pos (sample unit); 
				if(edge_table->tsl_tail == pos + total_size)
					edge_table->tsl_tail = pos;
				if(edge_table->tsl_head == pos + total_size)
					edge_table->tsl_head = pos;
					
				sample_node* pos_s = node_table->ID_to_pos(tmp.vice.s);
				sample_node* pos_d = node_table->ID_to_pos(tmp.vice.d);
				if (!pos_s)
				{
					pos_s = node_table->insert(tmp.vice.s);
					node_count++;
				}
				if (!pos_d)
				{
					pos_d = node_table->insert(tmp.vice.d);
					node_count++;
				}					// if the node is not in the table ,insert it

				link_list(pos_s, pos_d, tmp.vice.bi_direction, pos); // link the cross list;

				edge_table->table[pos].vice.reset(tmp.s, tmp.d, tmp.priority, tmp.timestamp, -1, -1, tmp.bi_direction); // the old sample unit, now candidate unit, serves only as a test unit, which exams if the new
				// sample unit is valid. It is not necessary to insert it into the time sequence list. 
				return return_result;
			}
			else  // if there is no vice edge
			{
				if (!(edge_table->table[pos].vice.timestamp < 0))
					cout << time <<' '<<edge_table->table[pos].timestamp<<' '<<edge_table->table[pos].vice.timestamp << endl;
				//assert(edge_table->table[pos].vice.timestamp<0);
				sample_node* old_s = node_table->ID_to_pos(edge_table->table[pos].s);
				sample_node* old_d = node_table->ID_to_pos(edge_table->table[pos].d);
				bool old_bi = edge_table->table[pos].bi_direction;
				edge_info return_result(edge_table->table[pos].s, edge_table->table[pos].d, edge_table->table[pos].timestamp, edge_table->table[pos].priority, old_bi);
				dismiss(old_s, old_d, old_bi, pos);
				valid_num[g]--;
				
				edge_table->table[pos].vice.reset(edge_table->table[pos].s, edge_table->table[pos].d, edge_table->table[pos].priority, edge_table->table[pos].timestamp, -1, -1, edge_table->table[pos].bi_direction);
				edge_table->delete_sample(pos);
				return return_result;

			}
		}
		else
			return edge_info(0, 0, 0, 0);
	}

	void slice_switch(int land_mark, int g) // group g arrives at a landmark, any test edge behind this landmark loses effect
	{
		int begin = g*group_size;
		int end = (g+1)*group_size;
		for (int i = begin; i<end; i++)
		{
			if (edge_table->table[i].vice.timestamp >= 0 && edge_table->table[i].vice.timestamp<land_mark) // a out-dated test edge
			{
				q_count[g] -= 1 / pow(2, int(-(log(1 - edge_table->table[i].vice.priority) / log(2))) + 1);
				edge_table->delete_vice(i);
				if (edge_table->table[i].timestamp >= 0) // if there is an invalid sample edge
				{
				//	sample_node* pos_s = node_table->ID_to_pos(edge_table->table[i].s);
					//sample_node* pos_d = node_table->ID_to_pos(edge_table->table[i].d);
				//	modify_triangle(pos_s, pos_d, +1);
					q_count[g] += 1 / pow(2, int(-(log(1 - edge_table->table[i].priority) / log(2))) + 1);
					valid_num[g]++;  // a new valid edge
				}
				else // if not this position is empty
				{
					q_count[g] += 1;
					edge_count[g]--;
				}
			}
		}
	}

	void group_switch(long long time)
	{
		while (time - land_mark[gswitch_iter] >= window_size)
		{
			assert(time - land_mark[gswitch_iter] < 2 * window_size);
			last_mark[gswitch_iter] = land_mark[gswitch_iter];
			land_mark[gswitch_iter] = land_mark[gswitch_iter] + window_size;
			slice_switch(last_mark[gswitch_iter], gswitch_iter);
			gswitch_iter++;
			if (gswitch_iter == group_num)
				gswitch_iter = 0;
		}
	}
	unsigned int valid_sample_num()
	{
		unsigned int sample_size = 0;
		for (int i = 0; i < group_num; i++)
			sample_size += valid_num[i];
		return sample_size;
	}
	unsigned int cardinality_estimate()
	{
		unsigned int edge_estimate = 0;
		for (int i = 0; i < group_num; i++)
		{
			int m = group_size;
			double alpha = 0.7213 / (1 + (1.079 / m));
			int group_card = (double(alpha * m * m) / (q_count[i]));
			if (group_card < 2.5 * m)
				group_card = -log(1 - double(edge_count[i]) / m) * m;
			if(edge_count[i]>0)
				edge_estimate += group_card * (double(valid_num[i]) / edge_count[i]);
		}
		return edge_estimate;
	}
};

