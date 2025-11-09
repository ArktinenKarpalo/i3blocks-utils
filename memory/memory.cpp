/*
This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include <format>
#include <iostream>
#include <ranges>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>

#define RED "#FF7373"
#define ORANGE "#FFA500"

using namespace std;

struct real_mem {
  long long memTotal, memAvailable;
};

void real_usage(struct real_mem *rm) {
  FILE *f = fopen("/proc/meminfo", "r");
  if (!f)
    exit(1);
  char buf[8192];
  int read = fread(buf, 1, sizeof(buf) - 1, f);
  if (!read || fclose(f))
    exit(1);
  buf[read + 1] = '\0';

  char *head = buf;
  for (;;) {
    char *tail = strchr(head, ':');
    if (!tail)
      break;
    *tail = '\0';
    long long value = strtoull(tail + 1, NULL, 10);
    if (strcmp(head, "MemTotal") == 0) {
      rm->memTotal = value * 1024;
    } else if (strcmp(head, "MemAvailable") == 0) {
      rm->memAvailable = value * 1024;
    }
    if (!(head = strchr(tail + 1, '\n')))
      break;
    head++;
  }
}

int main() {
  int warning = 50;
  int critical = 20;
  int hist_size = 5;
  int rate = 1;

  if (const char *env_var = std::getenv("warning"))
    warning = stoi(env_var);
  if (const char *env_var = std::getenv("critical"))
    critical = stoi(env_var);
  if (const char *env_var = std::getenv("hist_size"))
    hist_size = stoi(env_var);
  if (const char *env_var = std::getenv("rate"))
    rate = stoi(env_var);

  const vector<string> levels{" ", "▁", "▂", "▃", "▄", "▅", "▆", "▇"};

  vector<int> histogram(hist_size);
  int histogram_index = 0;

  while (true) {
    struct real_mem rm;
    real_usage(&rm);
    long long usage = rm.memTotal - rm.memAvailable;
    double percentage = 100 * ((double)usage / rm.memTotal);

    if (critical && percentage > critical) {
      cout << format("<span color='{}'>", RED);
    } else if (warning && percentage > warning) {
      cout << format("<span color='{}'>", ORANGE);
    } else {
      cout << "<span>";
    }

    histogram[(histogram_index + histogram.size() - 1) % histogram.size()] =
        (usage * 8) / rm.memTotal;

    for (auto u : histogram | views::drop(histogram_index))
      cout << levels[u];
    for (auto u : histogram | views::take(histogram_index))
      cout << levels[u];
    histogram_index = (histogram_index + 1) % histogram.size();

    const double gb = 1024 * 1024 * 1024;
    double usage_gb = usage / gb;
    double total_gb = rm.memTotal / gb;
    cout << format(" {:4.1f}G/{:4.1f}G ({}%)</span>", usage_gb, total_gb,
                   (int)percentage)
         << endl;

    sleep(rate);
  }
}
