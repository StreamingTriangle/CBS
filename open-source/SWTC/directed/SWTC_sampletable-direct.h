#include<iostream>
#include<vector>
#include<assert.h>
#include<math.h>

#ifndef setting
#include "../../Common/directed/directed-setting.h"
#define setting
#endif

using namespace std;

class SWTC_sampletable
{
public:
	EdgeTable* edge_table;
	NodeTable* node_table;
	int group_num;// edge table is divided into segments of group, each group has a q_count and a valid_num;
	int total_size; // total_size of the edge table;
	int group_size; // size of each group. total_size / group_num;
	int* edge_count;
	int node_count;
	long long trcount;
	int* valid_num;
	double* q_count;
	vector<int> direction_parameters[7];
	vector<int> bi_parameters[7];
	int triangle_direction;


	SWTC_sampletable(int s, int g, int triangle_direction_)
	{
		total_size = s;
		group_num = g;
		group_size = total_size / group_num;
		node_count = 0;
		trcount = 0;
		edge_table = new EdgeTable(s);
		node_table = new NodeTable(4, 2 * s);

		valid_num = new int[group_num];
		q_count = new double[group_num];
		edge_count = new int[group_num];

		for (int i = 0; i < group_num; i++)
		{
			valid_num[i] = 0;
			q_count[i] = group_size;
			edge_count[i] = 0;
		}

		triangle_direction = triangle_direction_;

		// type A
		direction_parameters[0].resize(6);
		direction_parameters[0] = { out, out, out, in, in, in };
		bi_parameters[0].resize(0);

		// type B
		direction_parameters[1].resize(2);
		direction_parameters[1] = { in, out };
		bi_parameters[1].resize(0);

		// type C
		direction_parameters[2].resize(4);
		direction_parameters[2] = { bi, out, in, bi };
		bi_parameters[2].resize(4);
		bi_parameters[2] = { out, in, in, out };

		// type D
		direction_parameters[3].resize(2);
		direction_parameters[3] = { out, bi };
		bi_parameters[3].resize(2);
		bi_parameters[3] = { in, in };

		// type E
		direction_parameters[4].resize(2);
		direction_parameters[4] = { bi, in };
		bi_parameters[4].resize(2);
		bi_parameters[4] = { out, out };

		// type F
		direction_parameters[5].resize(2);
		direction_parameters[5] = { bi, bi };
		bi_parameters[5].resize(8);
		bi_parameters[5] = { in, bi, bi, in, bi, out, out, bi };


		// type G
		direction_parameters[6].resize(0);
		bi_parameters[6].resize(2);
		bi_parameters[6] = { bi, bi };
	}
	~SWTC_sampletable()
	{
		delete edge_table;
		delete node_table;
		delete[]q_count;
		delete[]valid_num;
		delete[]edge_count;
	}

	void modify_triangle(sample_node* pos_s, sample_node* pos_d, bool bi_direction, int direction, int op)
	{
		//cout << "modify_triangle " << pos_s->nodeID << ' ' << pos_d->nodeID << ' ' << bi_direction << ' ' << op << endl;
		vector<unsigned int> v1;
		vector<unsigned int> v2;
		unordered_map<unsigned int, int> m1;
		unordered_map<unsigned int, int> m2;
		unsigned int s_num = pos_s->nodeID;
		unsigned int d_num = pos_d->nodeID;

		int direction_round = direction_parameters[direction].size() / 2;
		int bi_round = bi_parameters[direction].size() / 2;

		int round = bi_direction ? bi_round : direction_round;
		int direction_s;
		int direction_d;
		int last_direction_s = -1;
		int last_direction_d = -1;

		for (int i = 0; i < round; i++)
		{
			direction_s = bi_direction ? bi_parameters[direction][i * 2] : direction_parameters[direction][i * 2];
			direction_d = bi_direction ? bi_parameters[direction][i * 2 + 1] : direction_parameters[direction][i * 2 + 1];

			if (direction_s != last_direction_s)
			{
				v1.clear();
				m1.clear();
				int edge_s = pos_s->first_edge[direction_s];
				while (edge_s >= 0)
				{
					unsigned int tmp = (edge_table->table[edge_s].s == s_num ? edge_table->table[edge_s].d : edge_table->table[edge_s].s);
					if (edge_table->table[edge_s].vice.timestamp<0 || edge_table->table[edge_s].vice.timestamp>edge_table->table[edge_s].timestamp)
					{
						if (m1.find(tmp) == m1.end())
						{
							v1.push_back(tmp);
							m1[tmp] = 1;
						}
						else
							m1[tmp]++;
					}
					edge_s = (edge_table->table[edge_s].s == s_num ? edge_table->table[edge_s].pointers[last_s] : edge_table->table[edge_s].pointers[last_d]);

				}
			}
			if (direction_d != last_direction_d)
			{
				v2.clear();
				m2.clear();
				int edge_d = pos_d->first_edge[direction_d];
				while (edge_d >= 0)
				{
					unsigned int tmp = (edge_table->table[edge_d].s == d_num ? edge_table->table[edge_d].d : edge_table->table[edge_d].s);
					if (edge_table->table[edge_d].vice.timestamp<0 || edge_table->table[edge_d].vice.timestamp>edge_table->table[edge_d].timestamp) // only count the valid edge
					{
						if (m2.find(tmp) == m2.end())
						{
							v2.push_back(tmp);
							m2[tmp] = 1;
						}
						else
							m2[tmp]++;
					}
					edge_d = (edge_table->table[edge_d].s == d_num ? edge_table->table[edge_d].pointers[last_s] : edge_table->table[edge_d].pointers[last_d]);
				}
			}
			vector<unsigned int> common_neighbor;
			count_join(v1, v2, common_neighbor);
			for (int j = 0; j < common_neighbor.size(); j++)
			{
				int f1 = m1[common_neighbor[j]];
				int f2 = m2[common_neighbor[j]];
				int sum = f1 * f2 * op;
				trcount += sum;
			}

			common_neighbor.clear();
			vector<unsigned int>().swap(common_neighbor);
			last_direction_s = direction_s;
			last_direction_d = direction_d;
		}
		v1.clear();
		v2.clear();
		m1.clear();
		m2.clear();
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




	void insert(unsigned int s_num, unsigned int d_num, bool bi_direction, int g, double p, long long time, long long land_mark, long long last_mark)
	{
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
				assert(edge_table->table[pos].vice.timestamp < land_mark&& edge_table->table[pos].vice.timestamp >= last_mark);

				if (p >= edge_table->table[pos].vice.priority)
				{
					valid_num[g]++;
					edge_table->replace_sample(s_num, d_num, p, time, bi_direction, pos);
					link_list(pos_s, pos_d, bi_direction, pos);
					modify_triangle(pos_s, pos_d, bi_direction, triangle_direction, 1);
					q_count[g] = q_count[g] - 1 / pow(2, int(-(log(1 - edge_table->table[pos].vice.priority) / log(2))) + 1) + 1 / pow(2, int(-(log(1 - p) / log(2))) + 1);
					edge_table->delete_vice(pos);
				}
				else
				{
					edge_table->replace_sample(s_num, d_num, p, time, bi_direction, pos);
					link_list(pos_s, pos_d, bi_direction, pos); // remain invalid
				}

			}
			else {
				edge_count[g]++;
				valid_num[g]++;
				edge_table->replace_sample(s_num, d_num, p, time, bi_direction, pos);	// if there is no sampled edge in this substream, this is the first one
			//	cout << pos << endl;
				link_list(pos_s, pos_d, bi_direction, pos);
				modify_triangle(pos_s, pos_d, bi_direction, triangle_direction, 1);
				q_count[g] = q_count[g] - 1 + 1 / pow(2, int(-(log(1 - p) / log(2))) + 1);
			}
			return;
		}


		if (edge_table->table[pos].timestamp < land_mark)
		{
			if (p >= edge_table->table[pos].priority)// if larger than the sampled p, replace it;
			{
				if (!(edge_table->table[pos].vice.timestamp >= land_mark || edge_table->table[pos].vice.timestamp < 0))
				{
					cout << "error " << pos << ' ' << edge_table->table[pos].vice.timestamp << ' ' << land_mark << endl;
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
				modify_triangle(old_s, old_d, old_bi, triangle_direction, -1);
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

				modify_triangle(pos_s, pos_d, bi_direction, triangle_direction, 1);
			}
			else   // if smaller than the sampled p, check the vice edge;
			{
				if (p >= edge_table->table[pos].vice.priority) // can replace it;
					edge_table->replace_vice(s_num, d_num, p, time, bi_direction, pos);
			}
		}
		else  //the sample edge is larger than the landmark
		{

			if (p >= edge_table->table[pos].priority)// if larger than the sampled p, replace it;
			{
				// in this case, we need to check the vice edge,

				if (edge_table->table[pos].vice.timestamp < land_mark && edge_table->table[pos].vice.timestamp >= 0) // then this is a test edge
				{

					assert(edge_table->table[pos].vice.timestamp >= last_mark);
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
						modify_triangle(pos_s, pos_d, bi_direction, triangle_direction, 1);
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
						link_list(pos_s, pos_d, bi_direction, pos);
					}
				}
				else //else there should be no vice edge, we replace the sampled edge.
				{
					sample_node* old_s = node_table->ID_to_pos(edge_table->table[pos].s);
					sample_node* old_d = node_table->ID_to_pos(edge_table->table[pos].d);
					bool old_bi = edge_table->table[pos].bi_direction;

					q_count[g] = q_count[g] - 1 / pow(2, int(-(log(1 - edge_table->table[pos].priority) / log(2))) + 1) + 1 / pow(2, int(-(log(1 - p) / log(2))) + 1);

					modify_triangle(old_s, old_d, old_bi, triangle_direction, -1);

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

					modify_triangle(pos_s, pos_d, bi_direction, triangle_direction, 1);

				}

			}

		}

		return;
	}


	void update(long long time)  // when the sampled edge expires, delete it and move the candidate edge one rank upper. Before this function the cross lists including this pos should be changed, and after this function the new sampled edge (valid or not) should be 
		// added into the curresponding cross lists;
		//time = T-N
	{
		int tsl_pos = edge_table->tsl_head;
		if (tsl_pos < 0)
			return;
		int pos = tsl_pos % total_size;


		while (edge_table->table[pos].timestamp < time)
		{

			int g = pos / group_size;
			tsl_pos = edge_table->table[pos].time_list_next;


			if (edge_table->table[pos].vice.timestamp >= time)
			{
				sample_node* old_s = node_table->ID_to_pos(edge_table->table[pos].s);
				sample_node* old_d = node_table->ID_to_pos(edge_table->table[pos].d);
				bool old_bi = edge_table->table[pos].bi_direction;
				modify_triangle(old_s, old_d, old_bi, triangle_direction, -1);
				dismiss(old_s, old_d, old_bi, pos);

				/////////////////////////////////////////////////////////
				sample_unit tmp = edge_table->table[pos];

				edge_table->delete_sample(pos); // delete the expired sample;


				valid_num[g]--;  // the valid num decreases;


				edge_table->table[pos].reset(tmp.vice.s, tmp.vice.d, tmp.vice.priority, tmp.vice.timestamp, tmp.vice.time_list_prev, tmp.vice.time_list_next, tmp.vice.bi_direction); // the vice edge is an invalid sample now
				edge_table->set_time_list(tmp.vice.time_list_prev, 1, pos);
				edge_table->set_time_list(tmp.vice.time_list_next, 0, pos); // the pointer used to be pos + total_size (candidate unit), now updated to pos (sample unit); 
				if (edge_table->tsl_tail == pos + total_size)
					edge_table->tsl_tail = pos;
				if (edge_table->tsl_head == pos + total_size)
					edge_table->tsl_head = pos;




				sample_node* pos_s = node_table->ID_to_pos(tmp.vice.s);
				sample_node* pos_d = node_table->ID_to_pos(tmp.vice.d);
				bool bi_direction = tmp.vice.bi_direction;
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

				link_list(pos_s, pos_d, bi_direction, pos); // link the cross list;
				edge_table->table[pos].vice.reset(tmp.s, tmp.d, tmp.priority, tmp.timestamp, -1, -1, tmp.bi_direction);

			}
			else  // if there is no vice edge
			{
				sample_node* old_s = node_table->ID_to_pos(edge_table->table[pos].s);
				sample_node* old_d = node_table->ID_to_pos(edge_table->table[pos].d);
				bool old_bi = edge_table->table[pos].bi_direction;
				modify_triangle(old_s, old_d, old_bi, triangle_direction, -1);
				dismiss(old_s, old_d, old_bi, pos);
				valid_num[g]--;

				edge_table->table[pos].vice.reset(edge_table->table[pos].s, edge_table->table[pos].d, edge_table->table[pos].priority, edge_table->table[pos].timestamp, -1, -1, edge_table->table[pos].bi_direction);

				edge_table->delete_sample(pos);

			}

			if (tsl_pos < 0)
				break;
			pos = tsl_pos % total_size;

		}
	}

	void slice_switch(int land_mark, int g) // group g arrives at a landmark, any test edge behind this landmark loses effect
	{
		int begin = g * group_size;
		int end = (g + 1) * group_size;
		for (int i = begin; i < end; i++)
		{

			if (edge_table->table[i].vice.timestamp >= 0 && edge_table->table[i].vice.timestamp < land_mark) // a out-dated test edge
			{
				q_count[g] -= 1 / pow(2, int(-(log(1 - edge_table->table[i].vice.priority) / log(2))) + 1);
				edge_table->delete_vice(i);
				if (edge_table->table[i].timestamp >= 0) // if there is an invalid sample edge
				{
					sample_node* pos_s = node_table->ID_to_pos(edge_table->table[i].s);
					sample_node* pos_d = node_table->ID_to_pos(edge_table->table[i].d);
					bool bi_direction = edge_table->table[i].bi_direction;
					modify_triangle(pos_s, pos_d, bi_direction, triangle_direction, 1);
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
};
