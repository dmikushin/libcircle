#include <ghc/filesystem.hpp>
#include <iostream>
#include <libcircle.hpp>

namespace fs = ghc::filesystem;
using namespace std;

/* An example of a create callback defined by your program */
static void my_create_some_work(circle::handle *handle) {
  /*
   * This is where you should generate work that needs to be processed.
   * For example, if your goal is to size files on a cluster filesystem,
   * this is where you would read directory and and enqueue directory names.
   *
   * By default, the create callback is only executed on the root
   * process, i.e., the process whose call to circle::init returns 0.
   * If the circle::CREATE_GLOBAL option flag is specified, the create
   * callback is invoked on all processes.
   */

  const fs::path directory = "/bin/";
  if (fs::exists(directory) && fs::is_directory(directory)) {
    for (fs::directory_iterator i(directory), ie; i != ie; i++) {
      if (!fs::exists(i->status()) || !fs::is_regular_file(i->status()))
        continue;

      const string filename = i->path().string();
      vector<uint8_t> content(filename.begin(), filename.end());
      handle->enqueue(content);
    }
  }
}

static size_t sztotal = 0;

static void store_in_database(size_t finished_work) {
  sztotal += finished_work;
}

/* An example of a process callback defined by your program. */
static void my_process_some_work(circle::handle *handle) {
  /*
   * This is where work should be processed. For example, this is where you
   * should size one of the files which was placed on the queue by your
   * create_some_work callback. You should try to keep this short and block
   * as little as possible.
   */
  vector<uint8_t> content;
  handle->dequeue(content);
  string my_data(content.begin(), content.end());

  size_t finished_work = fs::file_size(my_data);

  store_in_database(finished_work);
}

int main(int argc, char *argv[]) {
  /*
   * Initialize state required by libcircle. Arguments should be those passed in
   * by the launching process. argc is a pointer to the number of arguments,
   * argv is the argument vector. The return value is the MPI rank of the
   * current process.
   */
  int rank = circle::init(argc, argv, circle::RuntimeFlags::DefaultFlags);

  circle::enable_logging(circle::LOG_INFO);

  /*
   * Processing and creating work is done through callbacks. Here's how we tell
   * libcircle about our function which creates the initial work. For MPI nerds,
   * this is your rank 0 process.
   */
  circle::cb_create(&my_create_some_work);

  /*
   * After you give libcircle a way to create work, you need to tell it how that
   * work should be processed.
   */
  circle::cb_process(&my_process_some_work);

  /*
   * Now that everything is setup, lets execute everything.
   */
  circle::begin();

  /*
   * Finally, give libcircle a chance to clean up after itself.
   */
  circle::finalize();

  cout << "Rank " << rank << " partial size = " << sztotal << endl;

  return 0;
}
