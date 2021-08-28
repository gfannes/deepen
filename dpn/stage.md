# Parse bullets

* Count nr of ` ` and `*` @30m
  * Should work for `.textile` and `.md`
  * Normal line has depth 0
  * Line before bullets will receive the aggregate
* Use same hierarchy computation as for titles @30m
  * This will normalize all depths
  * Do not reduce depth to 0 if it was > 0

# Parse `.textile` titles @30m

# Support `use_case` and `feature` @2h

* Add links from `use_case` folder to `feature`
  * Support for finding `feature/a/b.md` iso `../../feature/a/b.md` from `use_case/c/d.md` via gradual step-down

# Support for `.textile` and `.md` export

* Titles and bullets according to output type
* Convert markup to internal in-line format, eg in naft
  * In a single line
	  * verbatim
	  * link
	  * user reference
  * Over several lines
	  * code blocks
	  * quote
* Convert in-line format to output-specific formatting
