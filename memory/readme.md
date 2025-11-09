# Memory usage widget

Display memory usage based on `MemAvailable` in `/proc/meminfo`.

Also plots a histogram of the recent usage.

![](demo.png)

## Usage
```
[memory]
interval=persist
markup=pango
hist_size=10
critical=80
warning=70
rate=5
```
