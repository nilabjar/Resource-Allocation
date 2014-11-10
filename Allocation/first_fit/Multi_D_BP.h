#ifndef MULTI_BP
#define MULTI_BP

//#include <windows.h>

#include "BinPack.h"
#include <vector>
#include <functional>
#include <map>
#include <fstream>

using namespace std;



typedef vector<Item_Type> Item_Array;

class Multi_D_BP : public BinPack
{
public:


	Multi_D_BP (int range_max, float slack, int upper_bound, string expt_name);

	~Multi_D_BP ();

	void init_new_expt ();

	void first_fit ();

	void print_all_bins ();

	void initialize (int total_items);

	void initialize_bins (int total_bins);

	void analyze ();

	void sort (int sort_type);

	void get_items_from_file (int total_items);

	virtual void report ();


	void worst_fit ();

	void best_fit ();

	void balancing ();

	virtual int add_an_item (Item_Type item);

	virtual void remove_item (int index);

	void clean_bins ();

//	void print_header ();

private:

	/// randomly shuffle the items ....
	void shuffle ();

	/// variables required for balancing the items 
	Item_Array packed_items;

	/// Sorts the items ...
	void special_sort ();

	void sort_sub_range (Item_Array& sub_items);

	void save_to_file ();

/// the Bin structure
	struct Bin
	{
		vector <int> items;
	    Item_Type capacity;
		int current_rank_;
	};


	/// the typedef vector ...

	Item_Array items;


	typedef vector<Bin> Bin_Array;
	Bin_Array bins;

/// Current trial status
	bool current_trial;

	/// the total items ..
	int total_items_;

	/// the total bins	
	int total_bins_;

	/// No. of Successes
	int success;

	/// No. of failures
	int failure;

	/// Total no. of trials
	int trials;

	/// Total no whose size is greater 
	int size_greater;

	/// item size greater than the bins 
	int size_exceeded;

	/// contains the items which could not be packed
	vector<int> failed_items;

	void print_items ();

	/// input file 
	std::ifstream in;

	/// using file
	bool using_file;

	/// type of sorting to be used ...
	int sort_type;

/// 
	int not_allocable;

	/// finds an item confirming to a given rank ...
	int find_item_of_rank (int bin_index, int rank);

	std::map<int, int> res_distr_;

	/// size of the Bin
	int bin_size;

	/// The range from which items will be chosen
	int range_max_;

	/// rejection count
	long long rejection_count;

	/// allowed slack 
	float allowed_slack_;

	/// counter - counts the number of trials
	int counter;

	/// name of the experiment
	string name;

	/// float upper slack bound
	int upper_bound_;
};

class Sort_Scaler : public 
	binary_function <Item_Array::iterator,
	Item_Array::iterator,
	bool>
{
public: 
	bool operator ()(Item_Type front,Item_Type back);
};

class Sort_Large_D : public 
	binary_function <Item_Array::iterator,
	Item_Array::iterator,
	bool>
{
public: 
	Sort_Large_D (bool cpu) {cpu_ = cpu;}
	bool operator ()(Item_Type front,Item_Type back);

	bool cpu_;
};


class Sort_Largest_Val : public 
	binary_function <Item_Array::iterator,
	Item_Array::iterator,
	bool>
{
public: 
	bool operator ()(Item_Type front,Item_Type back);
};


class Sort_Multi : public 
	binary_function <Item_Array::iterator,
	Item_Array::iterator,
	bool>
{
public: 
	bool operator ()(Item_Type front,Item_Type back);
};

#endif