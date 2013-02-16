# mist - a safety checker for Petri Nets and extensions

## Compilation

To install (last step of optional):

     $ ./autogen.sh
     $ ./configure
     $ make
     $ make install 

To uninstall:     

     $ make uninstall


## Synopsis

     $ mist --[options] <input_file>

### options

<table>
  <tr>
    <td>backward</td><td>the backward algorithm with invariant pruning described in Laurent Van Begin PhD thesis</td>
  </tr>
  <tr>
    <td>ic4pn</td><td>the abstraction refinement algorithm described in Fundamenta Informaticae, 88(3), 2008</td>
  </tr>
  <tr>
    <td>tsi</td><td>the abstraction refinement algorithm described in Technique et Sciences Informatiques, 28(9), 2009</td>
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

## Input file format and further details

Further information can be found on the [wiki][mistpage]. 

## Reporting Problems 

Please report issues on [GitHub][issueslist].
 
## Further Questions

Contact the author, [Pierre Ganty][email].

[issueslist]:https://github.com/pierreganty/mist/issues
[homepage]:http://software.imdea.org/people/pierre.ganty/index.html
[mistpage]:https://github.com/pierreganty/mist/wiki
[email]:mailto:pierreganty@gmail.com
