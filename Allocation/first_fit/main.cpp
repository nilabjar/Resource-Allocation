#include <windows.h>
#include <TCHAR.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>

#include <algorithm>
#include <fstream>
#include <sstream>

#include "BinPack.h"
const float BinPack::BINSIZE = 100;

#include "Single_D_BinPack.h"
#include "Multi_D_BP.h"

#include "Time_Window_BP.h"

using namespace std;

void setup_experiment ();
void run_test ();
void move_file (string src_file,string dest_file);

int main (int argc, char* argv[])
{

	// get the arguments=, first
//	BinPack* bin_pack = new Single_D_BinPack ();

	if (argc == 10)
	{
		setup_experiment ();

		run_test ();

		exit (0);
	}

	if (argv[1][0] == 'w')
	{

		int bin_no = atoi (argv[2]);

		char sort_type = argv[3][0];

		char algorithm = argv[4][0];

		/// This is a time window input
		Time_Window_BP * tbp = 
			new Time_Window_BP (bin_no, sort_type, algorithm);

		tbp->initialise_jobs ();

		tbp->time_based_BP ();


		delete tbp;
		exit (0);
	}
	int	range_max = atoi (argv[1]);
	int slack = atoi (argv[7]);
	int upper_bound = atoi (argv[8]);

	float slack_percent = (float)slack/100;

	float upper_slack_bound = (float)upper_bound/100;

	BinPack* bin_pack = new Multi_D_BP (range_max,
		                                slack_percent,
										upper_bound,
										"name");

    if (argc != 9)
	{
		cout << "Please the proper parameters "
			<< "items bins trials sort_form file bp_algo"
			<< endl;
		exit (0);
	}

	// get the options 
	int total_items = 0; // doesnt matter now ....

	int total_trials = atoi (argv[3]);
	bool use_file = false;
	bool use_sort = false;

	if (argv[5][0] == 'f')
		use_file = true;

	if (argv[4][0] == 's')
		use_sort = true;

	for (int i = 0;i < total_trials;i++)
	{
		bin_pack->init_new_expt ();

		int total_bins = atoi (argv[2]);	
		bin_pack->initialize_bins (total_bins);

		if (use_file)
			bin_pack->get_items_from_file (total_items);
		else
		{
			// set up the items
			bin_pack->initialize (total_items);
		}


		// sort the items ....
//		if (use_sort)
			bin_pack->sort (argv[4][0]);

		// next call the first_fit
		if (argv[6][0] == 'f')
		{
			bin_pack->first_fit ();
		}
		else if (argv[6][0] == 'w')
			bin_pack->worst_fit ();
		else if (argv[6][0] == 'b')
			bin_pack->best_fit ();
		else if (argv[6][0] == 'l')
			bin_pack->balancing ();

		bin_pack->print_all_bins ();

		bin_pack->analyze ();
	}

	bin_pack->report ();

	delete bin_pack;
}

struct Each_Trial
{
	int slack;
	int upper_slack;
	int trials;
	int distr_param;
};

vector <Each_Trial> total_trials;

void setup_experiment ()
{
	// first read the config file and get the data 
	ifstream in ("config.txt");

	if (!in)
		return;

	stringstream input;
	string str;

	while (!in.eof ())
	{
		Each_Trial et;

		// get the lines , discard them ...
		in >> str;

		if (str[0] == '#')
			continue;

		// get the slack
		in >> str;
		int pos = str.find ('=');
		string value = str.substr (pos + 1);
		et.slack = atoi (value.c_str ());

		// get the upper slack
		in >> str;
		pos = str.find ('=');
		value = str.substr (pos + 1);
		et.upper_slack = atoi (value.c_str ());

		// get the trial
		in >> str;
		pos = str.find ('=');
		value = str.substr (pos + 1);
		et.trials = atoi (value.c_str ());

		// get the distribution range
		in >> str;
		pos = str.find ('=');
		value = str.substr (pos + 1);
		et.distr_param = atoi (value.c_str ());

		total_trials.push_back (et);
	}

	in.close ();
}

void run_single_experiment (char sort,
							int bins,
							char heur,
							bool use_file,
							Each_Trial et,
							string name);

void run_test ()
{

	for (int i =0;i < total_trials.size ();i++)
	{
		// run a single experiment ..
		// best fit , sort 
		char buf[20];
		sprintf (buf, "input_%d.txt", i);

		run_single_experiment ('s', 10, 'b', false, total_trials[i], "Sort_BestFit");

		run_single_experiment ('s', 10, 'f', true, total_trials[i], "Sort_FirstFit");

		run_single_experiment ('a', 10, 'b', true, total_trials[i], "Max_BestFit");

		run_single_experiment ('a', 10, 'f', true, total_trials[i], "Max_FirstFit");

		ifstream in ("input.txt", ios::binary);
		ofstream out (buf, ios::binary);

		std::copy(istreambuf_iterator<char> (in.rdbuf ()), 
			istreambuf_iterator<char> (), 
			ostream_iterator<char> (out));


		//while (!in.eof ())
		//{
		//	char ch;
		//	ch = in.get ();
		//	out.put (ch);
		//}
		//out.put(eof);
		
		in.close ();
		out.close ();

		remove ("input.txt");
	}
}

void run_single_experiment (char sort,
							int bins,  
							char heur,
							bool use_file,
							Each_Trial et,
							string name)
{
	float slack_percent = (float)et.slack/100;

	BinPack* bin_pack = 
		new Multi_D_BP (et.distr_param,
		slack_percent,
		et.upper_slack,
		name);

	for (int i = 0;i < et.trials;i++)
	{
		bin_pack->init_new_expt ();

		bin_pack->initialize_bins (bins);

		if (use_file)
			bin_pack->get_items_from_file (NULL/*not used now*/);
		else
		{
			// set up the items
			bin_pack->initialize (NULL/*Not used now*/);
		}


		// sort the items ....
		bin_pack->sort (sort);

		if (heur == 'f')
			bin_pack->first_fit ();
		else if (heur == 'w')
			bin_pack->worst_fit ();
		else if (heur == 'b')
			bin_pack->best_fit ();
		else if (heur == 'l')
			bin_pack->balancing ();

		bin_pack->print_all_bins ();

		bin_pack->analyze ();
	}

	bin_pack->report ();

	char buf [20];
	sprintf (buf, "failed_%c_%c_%d_%d", heur, sort, et.slack, et.distr_param);

	move_file ("failed.txt" ,buf);

	delete bin_pack;
}

void move_file (string src_file,string dest_file)
{
	ifstream in (src_file.c_str (), ios::binary);
	ofstream out (dest_file.c_str (), ios::binary);

	std::copy(istreambuf_iterator<char> (in.rdbuf ()), 
		istreambuf_iterator<char> (), 
		ostream_iterator<char> (out));
	
	in.close ();
	out.close ();

	remove (src_file.c_str ());
}