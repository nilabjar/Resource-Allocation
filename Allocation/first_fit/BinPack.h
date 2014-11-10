#ifndef BINPACKH
#define BINPACKH

/// The typedef of items 
struct Item_Type
{
	int id;
	float cpu;
	float memory;
	int rank;
	bool packed;
	int bin_no;
};

class BinPack
{
public:

	virtual ~BinPack () {};

	virtual void init_new_expt (){};

	virtual void first_fit (){};

	virtual void print_all_bins (){};

	virtual void initialize (int total_items){};

	virtual void initialize_bins (int total_bins){};

	virtual void analyze (){};

	virtual void sort (int sort_type){};

	virtual void get_items_from_file (int){};

	virtual void report (){};

	virtual void worst_fit (){};

	virtual void best_fit (){};

	virtual void balancing (){};

	static const float BINSIZE;

	/// required for time based BP
	///returns an id  needed to remove the item
	virtual int add_an_item (Item_Type item){return 0;};

	virtual void remove_item (int index){};

	virtual void clean_bins (){};
};


#endif