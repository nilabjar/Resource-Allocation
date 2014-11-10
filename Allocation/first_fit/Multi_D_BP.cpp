//#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>


#include "Multi_D_BP.h"


#define LARGE   75
#define MEDIUM  35
#define SMALL    0

#define ZERO_ONE 10
#define ONE_ZERO 20

bool Sort_Scaler::
operator ()(Item_Type front,
			Item_Type back)
{
	int front_size = front.cpu + front.memory;
	int back_size = back.cpu + back.memory;

	return front_size > back_size;
}


bool Sort_Large_D::
operator ()(Item_Type front,
			Item_Type back)
{
	if (cpu_)
		return front.cpu > back.cpu;
	else
		return front.memory > back.memory;
}

bool Sort_Largest_Val::
operator ()(Item_Type front,
			Item_Type back)
{

	int front_larger = std::max (front.cpu, front.memory);
	int back_larger = std::max (back.cpu, back.memory);

	return front_larger > back_larger;
}

bool Sort_Multi::
operator ()(Item_Type front,
			Item_Type back)
{
	int front_larger = std::max (front.cpu, front.memory);
	int back_larger = std::max (back.cpu, back.memory);

	if (front_larger >= 85 || back_larger >= 85)
		return front_larger > back_larger;

	int front_size = front.cpu * front.memory;
	int back_size = back.cpu * back.memory;

	return front_size > back_size;
}

Multi_D_BP::Multi_D_BP (int range_max, float slack, int upper_bound, string expt_name)
  : success (0),
  failure(0),
  trials (0),
  size_exceeded (0),
  using_file (false),
  not_allocable (0),
  rejection_count (0),
  range_max_ (range_max),
  allowed_slack_ (slack),
  upper_bound_ (upper_bound)
{
//	srand (time(NULL));
	in.open ("input.txt");
	counter = 100;
	name = expt_name;
}

Multi_D_BP::~Multi_D_BP ()
{
	in.close ();
}
void Multi_D_BP::init_new_expt ()
{
// clear the items and the bins 
	this->items.clear ();
	this->bins.clear ();
	this->current_trial = true;
	this->failed_items.clear ();
//	this->not_allocable = 0;
}

void Multi_D_BP::first_fit ()
{
	for (int i = 0;i < items.size ();i++)
	{
		bool item_fitted = false;
		if (items[i].packed == true)
		{
			bins[items[i].bin_no].items.push_back (i);
			continue;
		}

		for (int j = 0;j < bins.size ();j++)
		{
			if (bins[j].capacity.cpu >= items[i].cpu 
				&& bins[j].capacity.memory >= items[i].memory)
			{
				// fits in the bin 
				bins[j].items.push_back(i);
				bins[j].capacity.cpu -= items[i].cpu;
				bins[j].capacity.memory -= items[i].memory;

				item_fitted = true;
				items[i].packed = true;
				items[i].bin_no = j;
				break;
			}
			// does not fit, try the next bin
		}

		if (item_fitted == false) // items could not be fit 
		{
			// packing failure 
			this->current_trial = false;
			this->failed_items.push_back (i);
		}
	}
}

void Multi_D_BP::worst_fit ()
{
	int last_space = 0;
	int worst_space_bin = 0;

	for (int i = 0;i < items.size ();i++)
	{
		bool item_fitted = false;
		last_space = 0;
		worst_space_bin = 0;

		for (int j = 0;j < bins.size ();j++)
		{
			if (bins[j].capacity.cpu >= items[i].cpu 
				&& bins[j].capacity.memory >= items[i].memory)
			{
				// fits in the bin 

				// check the bin space ...
				int space = bins[j].capacity.cpu 
					+ bins[j].capacity.memory;

				if (space > last_space)
				{
					worst_space_bin = j;
					last_space = space;
				}

				item_fitted = true;
			}
			// does not fit, try the next bin
		}// for loop on bins ...

		if (item_fitted == false) // items could not be fit 
		{
			// packing failure 
			this->current_trial = false;
			this->failed_items.push_back (i);
		}
		else
		{
			// here fit the bin 
			bins[worst_space_bin].items.push_back(i);
			bins[worst_space_bin].capacity.cpu -= items[i].cpu;
			bins[worst_space_bin].capacity.memory -= items[i].memory;
		}
	}// for loop on items ....
}

void Multi_D_BP::best_fit ()
{
	int last_space = 0;
	int best_space_bin = 0;

	for (int i = 0;i < items.size ();i++)
	{
		bool item_fitted = false;
		last_space = 2*BINSIZE + 1;
		best_space_bin = 0;

		for (int j = 0;j < bins.size ();j++)
		{
			if (bins[j].capacity.cpu >= items[i].cpu 
				&& bins[j].capacity.memory >= items[i].memory)
			{			
				// fits in the bin 

				// check the bin space ...
				int space = bins[j].capacity.cpu 
					+ bins[j].capacity.memory;

				if (space < last_space)
				{
					best_space_bin = j;
					last_space = space;
				}

				item_fitted = true;
			}
			// does not fit, try the next bin
		}// for loop on bins ...

		if (item_fitted == false) // items could not be fit 
		{
			// packing failure 
			this->current_trial = false;
			this->failed_items.push_back (i);
		}
		else
		{
			// here fit the bin 
			bins[best_space_bin].items.push_back(i);
			bins[best_space_bin].capacity.cpu -= items[i].cpu;
			bins[best_space_bin].capacity.memory -= items[i].memory;
		}
	}// for loop on items ....
}

void Multi_D_BP::balancing ()
{

// rank the items first 

	for (int i =0;i < items.size ();i++)
	{
		if (items[i].cpu >= items[i].memory)
		{
			items[i].rank = ZERO_ONE;
		}
		else
		{
			items[i].rank = ONE_ZERO;
		}

		items[i].packed = false;
	}

	for (int i = 0;i < bins.size ();i++)
	{
		// find the first unpacked item and insert it into the bin
		int k = 0;
		for (k =0;k < items.size ();k++)
		{
			if (items[k].packed == false)
				break;
		}

		if (k == items.size ())
			return; /// all items packed ....

		// insert the item ...
		bins[i].items.push_back(k);
		bins[i].capacity.cpu -= items[k].cpu;
		bins[i].capacity.memory -= items[k].memory;
		items[k].packed = true;

		//update the rank of the bin
		if (bins[i].capacity.cpu <= bins[i].capacity.memory)
			bins[i].current_rank_ = ZERO_ONE;
		else
			bins[i].current_rank_ = ONE_ZERO;

		while (1)
		{
			int index = find_item_of_rank (i, bins[i].current_rank_);

			if (index == -1)
			{
				// find the item in the other rank 
				index = 
					find_item_of_rank (i,
					(bins[i].current_rank_ == ONE_ZERO)? ZERO_ONE:ONE_ZERO);

				if (index == -1) // go for the next bin , this bin cannot be filled ....
					break;
			}

			// insert the item in the bin .....
			bins[i].items.push_back(index);
			bins[i].capacity.cpu -= items[index].cpu;
			bins[i].capacity.memory -= items[index].memory;
			items[index].packed = true;

			//update the rank of the bin
			if (bins[i].capacity.cpu <= bins[i].capacity.memory)
				bins[i].current_rank_ = ZERO_ONE;
			else
				bins[i].current_rank_ = ONE_ZERO;
		}//while 1
	}

	// find the items that could not be packed ..
	for (int i =0;i < items.size ();i++)
	{
		if (items[i].packed == false)
		{
			this->current_trial = false;
			this->failed_items.push_back (i);
		}
	}
}

int Multi_D_BP::find_item_of_rank (int bin_index, int rank)
{
	// find an item which has same rank .....
	for (int i = 0;i < items.size ();i++)
	{
		if (items[i].rank == rank
			&& items[i].packed == false)
		{
			if (bins[bin_index].capacity.cpu >= items[i].cpu 
				&& bins[bin_index].capacity.memory >= items[i].memory)
			{
				return i;
			}
		}
	}
	return -1;
}

void Multi_D_BP::print_all_bins ()
{
	for (int i = 0;i < bins.size ();i++)
	{
		cout << "The items in Bin " 
             << i 
			 << " are :"
			 << endl;
		
		for (int j = 0;j < bins[i].items.size ();j++)
		{
			cout << "\t" << items[bins[i].items[j]].cpu 
				<< " , "
				<< items[bins[i].items[j]].memory
				<< endl;
		}
	}

	for (int i = 0;i < failed_items.size ();i++)
	{
		cout << "\n The failed items are "  << endl
			<< "\tCpu : " 
			<< items[failed_items[i]].cpu << endl
			<< "\tMemory : "
			<< items[failed_items[i]].memory
			<< endl;
	}
}
//
//void Multi_D_BP::initialize (int total_items)
//{
//	this->total_items_ = total_items;
//
//
//
//// init the bins and the items 
//
//
//	float size;
//// init the items , with random sizes ....
//
//
//	// create a copy of the bins 
//	Bin_Array temp_bins = this->bins;
//	int bin_index = 0;
//
//	for (int i = 0;i < total_items_;i++)
//	{
//		Item_Type item;
//		size = rand () % 100 + 1;
////		size = rand () % 65 + 1;
//		item.cpu = size;
//		item.packed = false;
//
//		size = rand () % 100 + 1;
////		size = rand () % 65 + 1;
//		item.memory = size;
//
//// now slice this from the bin .....
//		if (bin_index < this->total_bins_)
//		{
//			if (temp_bins[bin_index].capacity.cpu >item.cpu
//				&& temp_bins[bin_index].capacity.memory > item.memory)
//			{
//				temp_bins[bin_index].capacity.cpu -= item.cpu;
//				temp_bins[bin_index].capacity.memory -= item.memory;
//			}
//			else
//			{
//				// create a bin out of the slack left in the bin and then use the next bin from now
//				//			item.cpu = temp_bins[bin_index].capacity.cpu;
//				//			item.memory = temp_bins[bin_index].capacity.memory;
//				bin_index++;
//
//				if (bin_index >= this->total_bins_)
//					this->not_allocable++;
//				else
//				{
//					temp_bins[bin_index].capacity.cpu -= item.cpu;
//					temp_bins[bin_index].capacity.memory -= item.memory;
//				}
//			}
//		}
//
//		items.push_back (item);
//	}
//
//	// now fill the other dimension	
////	for (int i = 0;i < total_items_;i++)
////	{
////		size = rand () % 100 + 1;
//////		size = rand () % 65 + 1;
////		items[i].memory = size;
////	}
//
//	this->print_items ();
//
//	if (!using_file)
//		this->save_to_file ();
//}

void Multi_D_BP::initialize (int total_items)
{
	int size;
	Item_Type item;

	Bin_Array temp_bins = this->bins;

//	this->total_items_ = total_items;

	counter++;
	time_t now = time (NULL);
	now += counter;

	srand (counter);

	for (int i = 0;i < this->total_bins_;i++)
	{
		Item_Array tmp_items;

		while (true)
		{
			size = rand () % this->range_max_ + 1;
			item.cpu = size;
			item.packed = false;

			size = rand () % this->range_max_ + 1;
			item.memory = size;

			if ((temp_bins[i].capacity.cpu - this->upper_bound_) >= item.cpu
				&& (temp_bins[i].capacity.memory - this->upper_bound_) >= item.memory)
			{
				temp_bins[i].capacity.cpu -= item.cpu;
				temp_bins[i].capacity.memory -= item.memory;
			}
			else
			{
				// clean up the current bin ....
				temp_bins[i].capacity.cpu = this->bin_size;
				temp_bins[i].capacity.memory = this->bin_size;

				tmp_items.clear ();

				i--;
				rejection_count++;
				break;
			}

			//insert it into the item array
			tmp_items.push_back (item);

			int slack_cpu = temp_bins[i].capacity.cpu;

			int slack_mem = temp_bins[i].capacity.memory;

			float slack_percent_mem = (float)slack_mem/(this->bin_size);

			float slack_percent_cpu = (float)slack_cpu/(this->bin_size);

//			if (temp_bins[i].capacity.cpu < 1 || temp_bins[i].capacity.memory < 1 
			if (slack_percent_cpu < this->allowed_slack_
				&& slack_percent_mem < this->allowed_slack_)
			{
				this->items.insert (this->items.end (),
					tmp_items.begin (),
					tmp_items.end ());
				break;
			}
			else if (temp_bins[i].capacity.cpu < 1 || temp_bins[i].capacity.memory < 1)
			{
				// clean up the current bin ....
				temp_bins[i].capacity.cpu = this->bin_size;
				temp_bins[i].capacity.memory = this->bin_size;

				tmp_items.clear ();

				i--;
				rejection_count++;
				break;
			}

		}
	}

	this->print_items ();

	this->shuffle ();

	if (!using_file)
		this->save_to_file ();
}
/*
void Multi_D_BP::initialize (int total_items)
{

	int size;
	Item_Type item;

	Bin_Array temp_bins = this->bins;

//	this->total_items_ = total_items;

	for (int i = 0;i < this->total_bins_;i++)
	{
		while (true)
		{

			Item_Array tmp_items;
		
			size = rand () % this->bin_size + 1;
			item.cpu = size;
			item.packed = false;

			size = rand () % this->bin_size + 1;
			item.memory = size;

			if (temp_bins[i].capacity.cpu >= item.cpu
				&& temp_bins[i].capacity.memory >= item.memory)
			{
				temp_bins[i].capacity.cpu -= item.cpu;
				temp_bins[i].capacity.memory -= item.memory;
			}
			else
			{
				i--;
				break;
			}

			//insert it into the item array
			tmp_items.push_back (item);

			while (true)
			{
				int slack = temp_bins[i].capacity.cpu
					+ temp_bins[i].capacity.memory;

				float slack_percent = (float)slack/(2*this->bin_size);

				if (temp_bins[i].capacity.cpu < 1 || temp_bins[i].capacity.memory < 1)
					break;

				if (slack_percent < .02)
					break;

				size = rand () % (int)temp_bins[i].capacity.cpu + 1;
				item.cpu = size;
				item.packed = false;

				size = rand () % (int)temp_bins[i].capacity.memory + 1;
				item.memory = size;

				if (temp_bins[i].capacity.cpu >= item.cpu
					&& temp_bins[i].capacity.memory >= item.memory)
				{
					temp_bins[i].capacity.cpu -= item.cpu;
					temp_bins[i].capacity.memory -= item.memory;
				}

				//insert it into the item array
				this->items.push_back (item);
			}
		}
	}

	//	this->print_items ();

	this->shuffle ();

	if (!using_file)
		this->save_to_file ();

}
*/
void Multi_D_BP::shuffle ()
{
	// randomly select an item from the list and put
	Item_Array tmp_items;
	srand (time(NULL));

	int size;

	while (this->items.size ())
	{
		size = rand () % this->items.size ();

		tmp_items.push_back (this->items[size]);
		this->items.erase (this->items.begin () + size);
	}

	this->items = tmp_items;
}

void Multi_D_BP::initialize_bins (int total_bins)
{
	/// another trial 
	this->trials++;

	this->bin_size = 100;

	this->total_bins_ = total_bins;
	// now initialize the bins, with initial capacity as the bin size

	for (int i = 0;i < total_bins_;i++)
	{
		Bin a_new_bin;
		a_new_bin.capacity.cpu = BINSIZE;
		a_new_bin.capacity.memory = BINSIZE;
		bins.push_back (a_new_bin);
	}
}

void Multi_D_BP::analyze ()
{
// check the items sizes with the bin sizes ...

	int cpu_total_size = 0;
	int mem_total_size = 0;
	for (int i = 0;i < items.size ();i++)
	{
		cpu_total_size += items[i].cpu;
		mem_total_size += items[i].memory;
	}

	//cout << "The total item size is " 
	//	<< "cpu " << cpu_total_size
	//	<< " memory " << mem_total_size
	//	<< " The total bin size is "
	//	<< total_bins_*BINSIZE
	//	<< endl;

	if (cpu_total_size <= total_bins_*BINSIZE
		&& mem_total_size <= total_bins_*BINSIZE)
	{
		if (this->current_trial == false)
		{
			this->failure++;
			cout << "\n\tFAILURE" << endl;
		}
		else
		{
			this->success++;
			cout << "\tSUCCESS" << endl;
		}
	}
	else
		this->size_exceeded++;

	//cout << ((total_size > total_bins*BINSIZE) ? 
	//	"The items are larger than the total capacity" :
	//"The items are smaller than the total capacity")
	//	<< endl;

	// fill up the res_distr
	if (this->res_distr_.find (this->failed_items.size ()) == this->res_distr_.end ())
	{
		// add the number for the first time 
		this->res_distr_[failed_items.size ()] = 1;
	}
	else
	{
		this->res_distr_[failed_items.size ()]++;
	}

	ofstream out ("failed.txt", ios::app);

	for (int i = 0;i < failed_items.size ();i++)
	{
		out << items[failed_items[i]].cpu 
			<< ","
			<< items[failed_items[i]].memory
			<< endl;
	}

	out.close ();
}

void Multi_D_BP::sort (int sort_type)
{

	if (sort_type == 's')
		std::sort (this->items.begin (),
		this->items.end (),
		Sort_Scaler ());
	else if (sort_type == 'p')
		this->special_sort ();
	else if (sort_type == 'l')
	{
		this->sort_type = 'l';
		this->special_sort ();
	}

	else if (sort_type == 'a')
	{
		std::sort (this->items.begin (),
		this->items.end (),
		Sort_Largest_Val ());
	}
	else if (sort_type == 'm')
	{
		std::sort (this->items.begin (),
		this->items.end (),
		Sort_Multi ());
	}

//	this->print_items ();
}

void Multi_D_BP::get_items_from_file (int total_items)
{
	int cpu;
	int memory;

	this->using_file = true;

	if (!in)
		cout << "FATAL !!!! ERROR IN OPENING INPUT.txt" <<endl;

	in >> this->total_items_;

	for (int i =0;i < this->total_items_;i++)
	{
		in >> cpu;
		in >> memory;
		Item_Type a_item;

		a_item.cpu = cpu;
		a_item.memory = memory;

		items.push_back (a_item);
	}

//	this->shuffle ();
}

void Multi_D_BP::report ()
{
	ofstream res ("result.csv", ios::app);

//	cout << "Total Trials : " << this->trials << endl
	res << "Expt Name,"
		<< "Total Trials,"
		<< "Total success,"
		<< "Total failure,"
		<< "Total Size exceeded,"
		<< "Total Not Allocable,"
		<< "Total no of Rejects," 
		<< "Range of data," 
		<< "Allowed Slack" 		
		<< endl;

	res << this->name.c_str () << ","
		<< this->trials << ","
		<< this->success << ","
		<< this->failure << ","
		<< this->size_exceeded << ","
		<< this->not_allocable << ","
		<< this->rejection_count << ","
		<< this->range_max_ << ","
		<< this->allowed_slack_*100 << ","
		<< endl;

//	cout << "The result distribution is " <<endl;
//	res << "The result distribution is " <<endl;

	std::map<int,int>::iterator iter;

//	cout << "Failed\t Items" << endl;
//	res << "Failed,Items" << endl;
//
//	for (iter = res_distr_.begin ();
//		iter != res_distr_.end ();
//		iter++)
//	{
////		cout << iter->first 
//		res << iter->first 
//			<< ","
//			<< iter->second
//			<< endl;
//	}
	res.close ();
}

void Multi_D_BP::print_items ()
{
	ofstream out ("items.txt", ios::app);
	out << "New Trial : " 
		<< this->name.c_str ()
		<< " Slack " << this->allowed_slack_
		<< " Range " << this->range_max_ << " " 
		<< endl;

	for (int i = 0;i < items.size ();i++)
	{
		out << "The item " 
			<< i 
			<< " is " 
			<< items[i].cpu
			<< " , "
			<< items[i].memory
		<< endl;
	}

	out.close ();
}

void Multi_D_BP::special_sort ()
{
// first segregate the items into 3 classes 
// Large, Medium and Small

	Item_Array large;
	Item_Array medium;
	Item_Array small;

	for (int i = 0;i < this->items.size ();i++)
	{
		int size = items[i].cpu + items[i].memory;

		if (size > LARGE*2)
			large.push_back (items[i]);
		else if (size > MEDIUM*2)
			medium.push_back (items[i]);
		else
			small.push_back (items[i]);
	}

	// now work in each size region

	sort_sub_range (large);

	sort_sub_range (medium);

	sort_sub_range (small);

	// aggregate the ranges into the whole range 

	this->items.clear ();

	items = large;

	std::copy (medium.begin (),
		medium.end (),
		std::back_inserter (items));

	std::copy (small.begin (),
		small.end (),
		std::back_inserter (items));
}

void Multi_D_BP::sort_sub_range (Item_Array& sub_items)
{
	// first find out, whichever dimension has more demand on the 
	// resource , then sort based on that

	// in the large 

	if (sub_items.size () == 0)
		return;

	if (sort_type == 'l')
	{
		std::sort (sub_items.begin (),
		      sub_items.end (),
			  Sort_Largest_Val ());

		return;
	}

	int cpu_avg;
	int mem_avg;

	for (int i = 0;i < sub_items.size ();i++)
	{
		cpu_avg = sub_items[i].cpu;
		mem_avg = sub_items[i].memory;
	}

	if (cpu_avg > mem_avg)
		std::sort (sub_items.begin (),
		      sub_items.end (),
			  Sort_Large_D (true));
	else
		std::sort (sub_items.begin (),
		      sub_items.end (),
			  Sort_Large_D (false));
}

void Multi_D_BP::save_to_file ()
{
// save the item data to file for later use 
	ofstream out ("input.txt", ios::app);

	out << this->items.size () << endl;

	for (int i = 0;i < this->items.size ();i++)
	{
		out << items[i].cpu << "\t" 
			<< items[i].memory << std::endl;
	}
	out.close ();
}

int Multi_D_BP::add_an_item (Item_Type item)
{
	this->items.push_back (item);
	return (items.size () - 1);
}

void Multi_D_BP::remove_item (int index)
{
	int i;
	for (i = 0;i < items.size ();i++)
	{
		if (items[i].id == index)
		{
		// find out if this was packed , then remove it from the bin

			for (int j =0;j < bins.size ();j++)
			{
				int k;
				for(k = 0;k < bins[j].items.size ();k++)
				{
					if (bins[j].items[k] == i)
						break;
				}
				if (k != bins[j].items.size ())
				{
					bins[j].items.erase (bins[j].items.begin () + k);

					//replenish the system resources ....
					bins[j].capacity.cpu += items[i].cpu;
					bins[j].capacity.memory += items[i].memory;
				}
			}
			break;
		}
	}

	items.erase (items.begin () + i);
}

void Multi_D_BP::clean_bins ()
{
	for (int i = 0;i < bins.size ();i++)
	{
		bins[i].items.clear ();
	}
}
