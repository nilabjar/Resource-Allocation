#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>

#include "Single_D_BinPack.h"


Single_D_BinPack::Single_D_BinPack ()
{
}

void Single_D_BinPack::first_fit ()
{
	for (int i = 0;i < items.size ();i++)
	{
		for (int j = 0;j < bins.size ();j++)
		{
			if (bins[j].capacity >= items[i])
			{
				// fits in the bin 
				bins[j].items.push_back(i);
				bins[j].capacity -= items[i];
				break;
			}
			// does not fit, try the next bin
		}
	}
}

void Single_D_BinPack::print_all_bins ()
{
	for (int i = 0;i < bins.size ();i++)
	{
		cout << "The items in Bin " 
             << i 
			 << " are :"
			 << endl;
		
		for (int j = 0;j < bins[i].items.size ();j++)
		{
			cout << "\t" << items[bins[i].items[j]] << endl;
		}
	}
}

void Single_D_BinPack::initialize (int total_items)
{
	this->total_items_ = total_items;

// init the bins and the items 
	srand (time(NULL));

	float size;
// init the items , with random sizes ....

	for (int i = 0;i < total_items_;i++)
	{
		size = rand () % 100 + 1;
		items.push_back (size);
	}


	cout << "The items are : " << endl;

	for (int i = 0;i < items.size ();i++)
	{
		cout << "The item " 
			<< i 
			<< " is " 
			<< items[i]
		<< endl;
	}
}

void Single_D_BinPack::analyze ()
{
// check the items sizes with the bin sizes ...

	int total_size = 0;
	for (int i = 0;i < items.size ();i++)
	{
		total_size += items[i];
	}

	cout << "The total item size is " 
		<< total_size
		<< " The total bin size is "
		<< total_bins_*BINSIZE
		<< endl;

	cout << ((total_size > total_bins_*BINSIZE) ? 
		"The items are larger than the total capacity" :
	"The items are smaller than the total capacity")
		<< endl;
}

void Single_D_BinPack::sort ()
{
	// here sort the items .....
	//in descending order so use rbegin and rend
	std::sort (items.rbegin (), items.rend ());
}

void Single_D_BinPack::get_items_from_file ()
{
	ifstream in("items.txt");

	int item;
	while (!in.eof ())
	{
		in >> item;
		items.push_back (item);
	}
	in.close ();
}

void Single_D_BinPack::initialize_bins (int total_bins)
{
	this->total_bins_ = total_bins;
	// now initialize the bins, with initial capacity as the bin size

	for (int i = 0;i < total_bins_;i++)
	{
		Bin a_new_bin;
		a_new_bin.capacity = BINSIZE;
		bins.push_back (a_new_bin);
	}
}