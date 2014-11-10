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
void run_unsolvable_test (int argc, char*argv[]);

int main (int argc, char* argv[])
{

	// get the arguments=, first
//	BinPack* bin_pack = new Single_D_BinPack ();
	if (argv[1][0] == 'u')
	{
		// run unsolvable test 
		run_unsolvable_test (argc, argv);
		exit (0);
	}


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

	Multi_D_BP* bin_pack = new Multi_D_BP (range_max,
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

struct test_res
{
	int serial_no;
	int best_fit_sorted;
	int best_fit_max;
	int first_fit_sorted;
	int first_fit_max;
	int worst_fit_sort;
	int worst_fit_max;
	int can_be_fit;
};

struct res_summary
{
	int total_runs;
	int best_fit_sorted;
	int best_fit_max;
	int first_fit_sorted;
	int first_fit_max;
	int worst_fit_sort;
	int worst_fit_max;
	int can_be_fit;
	int heur_success;
};

void create_summary_report (res_summary& summary);
void create_summary_headings ();

void run_unsolvable_test (int argc, char*argv[])
{

	if (argc != 8)
	{
		cout << "Usage : first_fit u <bins> <trials> <solvable=s, unsolvable=u> <range> <lower_slack> <upper_slack>" << endl;
		exit (0);
	}

	int	range_max = atoi (argv[5]);
	int slack = atoi (argv[6]);
	int upper_bound = atoi (argv[7]);

	float slack_percent = (float)slack/100;

	float upper_slack_bound = (float)upper_bound/100;

	Multi_D_BP* bin_pack = new Multi_D_BP (range_max,
		                                slack_percent,
										upper_bound,
										"name");

	// get the options 
	int total_items = 0; // doesnt matter now ....

	int total_trials = atoi (argv[3]);
	bool solvable = false;

	if (argv[4][0] == 's')
		solvable = true;

	int total_bins = atoi (argv[2]);	
//	bin_pack->generate_items_unsolvable (total_bins);

// create title of report
	create_summary_headings ();
	
// initiate the report counters 

	res_summary summary;

	summary.best_fit_max = 0;
	summary.best_fit_sorted = 0;
	summary.first_fit_max = 0;
	summary.first_fit_sorted = 0;
	summary.worst_fit_max = 0;
	summary.worst_fit_sort = 0;
	summary.total_runs = total_trials;
	summary.can_be_fit = 0;
	summary.heur_success = 0;

	ofstream fill_char ("fillin_char.csv");

	for (int i = 0;i < total_trials;i++)
	{
		bin_pack->init_new_expt ();

		bin_pack->initialize_bins (total_bins);

		if (solvable)
			bin_pack->initialize (total_items);
		else
			bin_pack->generate_items_unsolvable (total_bins);

		bool s_ff, s_bf, a_ff, a_bf, cp, s_wf, a_wf;

		cp = false;

		// sort the items ....
			bin_pack->sort ('s');

		// next call the first_fit
			bin_pack->first_fit ();

			s_ff = bin_pack->current_result ();

			if (s_ff)
				bin_pack->print_filled_bins_char (fill_char,i+1,"Sort_FF");

			bin_pack->init_new_algo ();
			bin_pack->worst_fit ();

			s_wf = bin_pack->current_result ();

			if (s_wf)
				bin_pack->print_filled_bins_char (fill_char,i+1,"Sort_WF");

			bin_pack->init_new_algo ();
			bin_pack->best_fit ();

			s_bf = bin_pack->current_result ();

			if (s_bf)
				bin_pack->print_filled_bins_char (fill_char,i+1,"Sort_BF");

//			bin_pack->balancing ();

		// sort the items ....
			bin_pack->sort ('a');

		// next call the first_fit
			bin_pack->init_new_algo ();
			bin_pack->first_fit ();

			a_ff = bin_pack->current_result ();

			if (a_ff)
				bin_pack->print_filled_bins_char (fill_char,i+1,"Max_FF");

			bin_pack->init_new_algo ();
			bin_pack->worst_fit ();

			a_wf = bin_pack->current_result ();

			if (a_wf)
				bin_pack->print_filled_bins_char (fill_char,i+1,"Max_WF");

			bin_pack->init_new_algo ();
			bin_pack->best_fit ();

			a_bf = bin_pack->current_result ();

			if (a_bf)
				bin_pack->print_filled_bins_char (fill_char,i+1,"Max_BF");

//			bin_pack->balancing ();

			if (!s_ff && !s_bf && !a_ff && !a_bf && !s_wf && !a_wf)
			{
				if (!solvable)
				{
					bin_pack->init_new_algo ();
					cp = bin_pack->complete_pack ();
					cout << "Complete Pack " << ((cp)? "Success" : "Failure") << endl;
					if (cp) summary.can_be_fit++;
				}
			}
			else
			{
				cout << "Heuristic Success" << endl;
				summary.heur_success++;
			}

			//if (!(!s_ff && !s_bf && !a_ff && !a_bf ))
			//	cout << "Heuristic Success" << endl;

			test_res rec;

			rec.serial_no = i+1;
			rec.best_fit_max = a_bf;
			rec.best_fit_sorted = s_bf;
			rec.first_fit_max = a_ff;
			rec.first_fit_sorted = s_ff;
			rec.can_be_fit = cp;
			rec.worst_fit_max = a_wf;
			rec.worst_fit_sort = s_wf;

			int total_cpu = 0;
			int total_mem = 0;

			bin_pack->total_dimension_size (total_cpu, total_mem);

			ofstream out("unsolve_det.csv", ios::app);
			out << rec.serial_no 
				<< ","
				<< rec.best_fit_max 
				<< ","
				<< rec.best_fit_sorted
				<< ","
				<< rec.first_fit_max
				<< ","
				<< rec.first_fit_sorted
				<< ","
				<< rec.worst_fit_max
				<< ","
				<< rec.worst_fit_sort
				<< ","
				<< rec.can_be_fit
				<< ","
				<< bin_pack->back_tracks ()
				<< ","
				<< bin_pack->total_items ()
				<< ","
				<< bin_pack->back_track_level ()
				<< ","
				<< total_cpu
				<< ","
				<< total_mem
				<< endl;
			out.close();

//get the result

			if (a_bf) summary.best_fit_max++;
			if (s_bf) summary.best_fit_sorted++;
			if (a_ff) summary.first_fit_max++;
			if (s_ff) summary.first_fit_sorted++;
			if (a_wf) summary.worst_fit_max++;
			if (s_wf) summary.worst_fit_sort++;

	}

	fill_char.close ();

	create_summary_report (summary);

	delete bin_pack;
}

void create_summary_report (res_summary& summary)
{
	
	ofstream outdet("unsolve_det.csv", ios::app);
	outdet << summary.total_runs
		<< ","
		<< summary.best_fit_max
		<< ","
		<< summary.best_fit_sorted
		<< ","
		<< summary.first_fit_max
		<< ","
		<< summary.first_fit_sorted
		<< ","
		<< summary.worst_fit_max
		<< ","
		<< summary.worst_fit_sort
		<< ","
		<< summary.can_be_fit
		<< ","
		<< ""
		<< ","
		<< summary.heur_success
		<< endl;

	outdet.close();

	ofstream out_sum ("unsolve_sum.csv");
	out_sum << "Total Trials = "
		<< summary.total_runs
		<< endl
		<< "Best Fit Max = "
		<< summary.best_fit_max
		<< endl
		<<  "Best Fit Sorted = "
		<< summary.best_fit_sorted
		<< endl
		<<  "First Fit Max = "
		<< summary.first_fit_max
		<< endl
		<<  "First Fit Sorted = "
		<< summary.first_fit_sorted
		<< endl
		<<  "Worst Fit Max = "
		<< summary.worst_fit_max
		<< endl
		<<  "Worst Fit Sorted = "
		<< summary.worst_fit_sort
		<< endl
		<< "Complete Packing = "
		<< summary.can_be_fit 
		<< endl
		<< "Heuristic Succeeded = "
		<< summary.heur_success
		<< endl;

	out_sum.close ();
}

void create_summary_headings ()
{
	ofstream out("unsolve_det.csv");
	out << "Run Num." 
		<< ","
		<< "Best Fit Max"
		<< ","
		<< "Best Fit Sorted"
		<< ","
		<< "First Fit Max"
		<< ","
		<< "First Fit Sorted"
		<< ","
		<< "Worst Fit Max"
		<< ","
		<< "Worst Fit Sorted"
		<< ","
		<< "Allocable"
		<< ","
		<< "Back Tracks"
		<< ","
		<< "Total Items"
		<< ","
		<< "Back Track Level"
		<< ","
		<< "Total CPU"
		<< ","
		<< "Total Memory"
		<< endl;
	out.close();
}