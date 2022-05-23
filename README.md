# mist - a safety checker for Petri Nets and extensions

mist is tool to check safety properties against Petri Net like models.
It implements a variety of search algorithms described on the [wiki][mistpage].
Even though mist has its own input format there it can be translated from other formats 
(using, for instance, [tttrans](https://github.com/pevalme/bfc_fork))
mist also ships with Python scripts to run mist over sets of benchmarks
and an infrastructure producing bar graphs using [d3js](http://d3js.org/) with runtime and memory consumption information.

## Downloading Mist 

For ease of use, pre-built binaries are available using these links :

* Linux : https://github.com/yanntm/mist/blob/linux/mist_linux.tar.gz?raw=true
* OSX : https://github.com/yanntm/mist/blob/osx/mist_osx.tar.gz?raw=true
* Windows : https://github.com/yanntm/mist/blob/windows/mist_windows.tar.gz?raw=true

## Building mist on Unix

Type `./configure`, then `make`.

If `./configure` does not exist (you downloaded from Github), run `autoreconf -i` first.

`make install` will build and install mist (under `/usr/local` by default).

`./configure` accepts the usual options; see `INSTALL` for more details.

`make uninstall` will uninstall mist.

## Synopsis

     $ mist --algorithm --[option] <input_file>
     $ mist --help
     $ mist --version

### Algorithm
| Algorithm:  | Description of the algorithm |
|-------------|-----------------------------------------------------------------------------------------------------------|
| backward    | the backward algorithm with invariant pruning described in Laurent Van Begin PhD thesis                   |
| ic4pn	    | the abstraction refinement algorithm described in Fundamenta Informaticae, 88(3), 2008                    |
| tsi	    | the abstraction refinement algorithm described in Technique et Sciences Informatiques, 28(9), 2009        |
| eec	    | Expand, Enlarge and Check algorithm described in the Journal of Computer and System Sciences, 72(1), 2005 |
| eec-cegar   | Expand, Enlarge and Check with a counter-example based refinement (not described anywhere)                |

### Options
| Option      | Effect of the option                                                                             |
|-------------|-----------------------------------------------------------------------------------------------------------|
| timeout <*T*>   | establish an execution timeout of *T* seconds. Atfer that time the execution will be stopped and proper message will be shown                 |
| graph <*filename*>  | creates a file called *filename* to store the data needed to plot graphs of the memory usage and execution time of the given algorithm |
| verbose <*V*> | establish a verbosity level *V*
| help        | this help                                                                                                 |
| version     | show version number                                                                                       |


## Input file format and further details

Further information can be found on the [wiki][mistpage].

## Reporting Problems

Please report issues on [GitHub][issueslist].

## Further Questions

Contact the author, [Pierre Ganty][email].

## Requirements

You need to install [mustache][mustachepage] to generate the graphs with our script.

Tested under linux (Ubuntu 14.10) and MacOS (OSX 10.9)

[issueslist]:https://github.com/pierreganty/mist/issues
[homepage]:http://software.imdea.org/people/pierre.ganty/index.html
[mistpage]:https://github.com/pierreganty/mist/wiki
[email]:mailto:pierreganty@gmail.com
[mustachepage]:https://github.com/janl/mustache.js/
