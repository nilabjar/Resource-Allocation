#include <time.h>
#include <vector>
#include <functional>
#include <string>

typedef enum {start, end} EventType;

struct Job
{
	int id;
	int cpu;
	int mem;
	std::string name;
};

struct Job_Event
{
	Job *jb;
	int tm;
	EventType ev;
};

typedef std::vector<Job_Event> Job_List;

class Time_Window_BP
{

public:

	Time_Window_BP (int no_of_bins,
		char sort_type, 
		char algorithm);

	void initialise_jobs ();

	void time_based_BP ();

	void sort_jobs ();

	void allocate_jobs ();

private:

	int total_bins_;
	char sort_type_;
	char algorithm_;

	Job_List jobs_;
};

class Sort_Job_Times : public 
	std::binary_function <Job_List::iterator,
	Job_List::iterator,
	bool>
{
public: 
	bool operator ()(Job_Event front,Job_Event back);
};