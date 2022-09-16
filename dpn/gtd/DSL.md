# Domain-specific language

The idea is to indicate DPN metadata at the start of a heading, paragraph, bullet, node, ... This should work with [markdown](https://www.markdownguide.org/), [freeplane](https://docs.freeplane.org/#/) and [naft](https://github.com/gfannes/gubg.io/blob/master/src/gubg/parse/naft/spec.md). Additional metadata that might be supported by the format at hand can be interpreted and should not be lost when updating.

# State

* `.?`: New Node, considered unknown and should be explored.
* `.>`: Delegated to somebody else. This Node is now on the _waiting for_ list.
* `.@`: Node is actionable. This Node is now on the _actionables_ list.
* `.!`: Work in progress
* `..`: Locally done: for composite Nodes, this means all the work at this level is done, but sublevels might still require work.
* `.~`: Cancelled. Same as actually deleting it, but still present in the file.

# Due date

* `?YYYYMMDD`

# Priority

See RICE system: reach, impact, confidence, effort

* `!RIC`:
  * Reach: number of people affected
  * Impact: aAbBcCdDeEfF..., where each letter increases the impact.
	  * A: 2**0, B: 2**1, ...
	  * a: 0.5, b = (A+B)/2, ...
	* Confidence:
		* 0: 0%
		* 1: 10%
		* 9: 90%
		* a/A/<nothing>: 100%

# Moscow

# Sequence
* `.a`: Any sequence, no finish-to-start dependencies between childs of this Node
* `.n`: Finish-to-start dependencies between childs of this Node [default]

# Effort

* `$1.2`: 1.2 hours
* .@ Support for wdh format
** `$1w2d3h`: 3+2*8+1*5*8 hours

# Command

* `&command:argument`
** .? How can you indicate an include should be added at the top level? There is for .md no mechanism to close a Heading.
* `&include()`
** .. $1h Working with wildcards
** .. $1d Comma-separated list of files

# Metadata

* Best not to use `@`: this might be used in a normal sentence in the beginning: `@geert: please do this...`
* `^key:value`

# Report

* `%whatever`