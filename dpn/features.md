# ^my:dpn Features

## Input formats
### Markdown
* .@ Support line number
### Naft
* .@ Support line number
### Freemind
* .. Implement gubg/xml/Reader
### .? Source code
* Parse source code line by line and detect commend line as whitespace+comment chars
* Support using wildcards in `&include()`
* .? Do we need to activate/disable file extensions to reduce loading times?
* Support line numbers

## GTD lists
* Static
* .@ Interactive
** Provide inspection via TUI
*** Select TUI framework
**** [ftxui](https://github.com/ArthurSonzogni/FTXUI) was used in proast
*** Toggle list
*** 2-pane: list + node view

## Metadata
* .. Aggregate costs at different levels
* .. Change &include: to &include()
* .. Aggregate metadata through Nodes and Files

## !a Features, Urgency, RICE
* A Feature is something that has an external value, eg, interest by a customer. Adding an Urgency to a Node will make `dpn` interpret it as a Feature.
* Urgency is the multiplication of `reach`, `impact` and `confidence` and is indicated as `!<reach><impact><confidence>`
** Optional `reach` indicates the number of people impacted by a feature, default is 1
** .@ $1q Mandatory `impact` indicates the impact a certain feature has. This is a letter scale where the downcase are powers of 2, and upcase increases the impact with 50%:
*** a=1, b=2, c=4, d=8, ...
*** A=1.5, B=3, C=6, D=12, ...
** Optional `confidence` is a number between 0 and 1, default is 1
* When two tasks with the same Urgency have different completion Effort, it makes sense to focus on the task with the smaller completion Effort. This is what RICE indicates: Urgency/Effort.
* The RICE indicator is only computed for Nodes that have an Urgency specified: the accumulated Effort is used there, and not the Effort of all the descendant Nodes.

## .@ !a $2h Priority
Similar to Urgency, Nodes can have a [MoSCoW](https://en.wikipedia.org/wiki/MoSCoW_method) Priority:
* `must`: Without this item, the higher-level Feature makes no sense
* `should`: This item is expected to be part of the higher-level Feature
* `could`: This item is considered as optional, but still benificial
* `wont`: This item does not impact the value of the higher-level Feature
$2h The Priority is indicated as `.[mscw]` and is inherited from parents when unset
* Since a Node can have multiple parents via the `&include()` mechanism, the Priority is effectively a bitmask

## Configuration
* .. Load default inputs from config file
* .@ $2h Use gubg/naft/Reader to load Config
* .? Support specifying an editor for the different file formats
** With line number

## Options
* .? Support specifying the verb after the options

## Sublime
### Commands
* .? Support for setting/altering the state, cost, duedate, prio
* .? Support for navigating includes
### Syntax highlighting
* .@ Set color depending on state
* .@ Set highlighter as link

## QC
* .@ Add UTs
* .@ Add QCs
** Use `naft --pack` to handle files and folders