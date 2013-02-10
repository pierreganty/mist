# mist #

---

Safety checker for Petri Nets (and some extensions). Find more details (e.g.
the input format) [here][inputformat]. 

## Compilation ##

`./autogen.sh`

`./configure`

`make`

`make install` (optional)

To uninstall: `make uninstall`


## Name ##

`mist`, safety checker for Petri Nets and extensions.

## Synopsis ##

`mist --[options] <input_file>`

### Options ###

<table>
  <tr>
    <td>backward</td><td>the backward algorithm with invariant pruning described in Laurent Van Begin PhD thesis</td>
  </tr>
  <tr>
    <td>ic4pn</td><td>the algorithm described in Fundamenta Informaticae, 88(3), 2008</td>
  </tr>
  <tr>
    <td>tsi</td><td>the algorithm described in Technique et Sciences Informatiques, 28(9), 2009</td>
  </tr>
  <tr>
    <td>eec</td><td> Expand, Enlarge and Check algorithm described in the
 Journal of Computer and System Sciences, 72(1), 2005</td>
  </tr>
  <tr>
    <td>help</td><td>this help</td>
  </tr>
  <tr>
    <td>version</td><td>show version number</td>
  </tr>
</table>
 
## Further Questions ##

* More details including the [algorithms and input format][inputformat].
* Contact the author, [Pierre Ganty][email].

[homepage]:http://software.imdea.org/people/pierre.ganty/index.html
[inputformat]:http://software.imdea.org/~pierreganty/ist.html
[email]:mailto:pierreganty@gmail.com
