#include "Time_Window_BP.h"
#include <algorithm>
#include "BinPack.h"
#include "Multi_D_BP.h"
#include <iostream>

bool Sort_Job_Times::
operator ()(Job_Event front,
			Job_Event back)
{
	return front.tm > back.tm;
}

Time_Window_BP::Time_Window_BP (int no_of_bins,
								char sort_type, 
								char algorithm)
								: total_bins_ (no_of_bins),
								sort_type_(sort_type),
								algorithm_(algorithm)
{
}

void Time_Window_BP::initialise_jobs ()
{
	// create some jobs randomly ...

	// now create a few fixed jobs 
	// and then later do it randomly

	// A

  // read  the file 
	std::ifstream in ("jobs.txt");

	if (!in)
	{
		cout << "Jobs.txt not found" <<endl;
		exit (1);
	}

	int total_jobs = 1;

	while (!in.eof ())
	{
		Job* new_job = new Job;

		new_job->id = total_jobs;

		std::string name;
		in >> new_job->name;

		// create a start event 
		Job_Event js;
		js.ev = start;
		js.jb = new_job;

		in >> js.tm;

		this->jobs_.push_back (js);

// create a end event 
		Job_Event je;
		je.ev = end;
		je.jb = new_job;

		in >> je.tm;

		in >> new_job->cpu;
		in >> new_job->mem;

		this->jobs_.push_back (je);

		total_jobs++;
	}

	in.close ();

/*	Job* A = new Job;
	A->id = 1;
	Job* B = new Job;
	B->id = 2;
	Job* C = new Job;
	C->id = 3;
	Job* D = new Job;
	D->id = 4;
	Job* E = new Job;
	E->id = 5;
	Job* F = new Job;
	F->id = 6;

	Job_Event Ast,Aed, Bst, Bed, Cst, Ced, Dst, Ded, Est, Eed, Fst, Fed;

	Ast.ev = start;
	Ast.tm = 0;
	Ast.jb = A;

	this->jobs_.push_back (Ast);

	A->cpu = 65;
	A->mem = 81;

	Aed.ev = end;
	Aed.tm = 10;
	Aed.jb = A;

	this->jobs_.push_back (Aed);

	Bst.ev = start;
	Bst.tm = 3;
	Bst.jb = B;

	this->jobs_.push_back (Bst);

	B->cpu = 27;
	B->mem = 24;

	Bed.ev = end;
	Bed.tm = 14;
	Bed.jb = B;

	this->jobs_.push_back (Bed);

	Cst.ev = start;
	Cst.tm = 11;
	Cst.jb = C;

	this->jobs_.push_back (Cst);

	C->cpu = 77;
	C->mem = 18;

	Ced.ev = end;
	Ced.tm = 23;
	Ced.jb = C;

	this->jobs_.push_back (Ced);

	Dst.ev = start;
	Dst.tm = 16;
	Dst.jb = D;

	D->cpu = 80;
	D->mem = 40;

	this->jobs_.push_back (Dst);

	Ded.ev = end;
	Ded.tm = 30;
	Ded.jb = D;

	this->jobs_.push_back (Ded);

	Est.ev = start;
	Est.tm = 19;
	Est.jb = E;

	this->jobs_.push_back (Est);

	E->cpu = 34;
	E->mem = 12;

	Eed.ev = end;
	Eed.tm = 25;
	Eed.jb = E;

	this->jobs_.push_back (Eed);

	Fst.ev = start;
	Fst.tm = 27;
	Fst.jb = F;

	this->jobs_.push_back (Fst);

	F->cpu = 57;
	F->mem = 96;

	Fed.ev = end;
	Fed.tm = 33;
	Fed.jb = F;

	this->jobs_.push_back (Fed);
*/
}

void Time_Window_BP::
time_based_BP ()
{
	sort_jobs ();

	//allocate the jobs ...
	allocate_jobs ();
}

void Time_Window_BP::sort_jobs ()
{
	std::sort (this->jobs_.rbegin (),
		this->jobs_.rend (),
		Sort_Job_Times ());
}

void Time_Window_BP::allocate_jobs ()
{
	// parse through the job set according to event times
	// and do accordingly

	BinPack* bp = new Multi_D_BP (100/*use 100 as a default range*/, 0.10, 0.05, "name");

	// initialize the bin pack
	bp->init_new_expt ();

	bp->initialize_bins (this->total_bins_);
	bool alloc = false;

	for (int i = 0;i < this->jobs_.size ();i++)
	{
		if (jobs_[i].ev == start)
		{
			Item_Type it;
			it.cpu = jobs_[i].jb->cpu;
			it.memory = jobs_[i].jb->mem;
			it.packed = false;
			it.id = jobs_[i].jb->id;
			bp->add_an_item (it);
			alloc = false;
		}
		else if (jobs_[i].ev == end)
		{
			if (alloc == false)
			{
				bp->sort (this->sort_type_);
				bp->clean_bins ();

				switch (this->algorithm_)
				{
				case 'b':
					bp->best_fit ();
					break;
				case 'f':
					bp->first_fit ();
					break;
				case 'l':
					bp->balancing ();
					break;
				}

				bp->print_all_bins ();
				bp->analyze ();
				alloc = true;
			}
			bp->remove_item (jobs_[i].jb->id);
		}
	}

}
