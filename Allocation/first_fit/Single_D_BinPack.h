
#include "BinPack.h"
#include <vector>

using namespace std;

struct Bin
{
	vector<int> items;
	float capacity;
};

class Single_D_BinPack : public BinPack
{
public:

	Single_D_BinPack ();

	void first_fit ();

	void print_all_bins ();

	void initialize (int total_items);

	void initialize_bins (int total_bins);

	void analyze ();

	void sort ();

	void get_items_from_file ();

private:

	/// the typedef vector ...
	typedef vector<float> Item_Array;
	Item_Array items;


	typedef vector<Bin> Bin_Array;
	Bin_Array bins;

	/// the total items ..
	int total_items_;

	/// the total bins	
	int total_bins_;
};