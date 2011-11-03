Generator
=========

The AML can be read and used by many tools.

List of possible tools:
  - GUI editor (which check for API breakage?) which allow handling?
  - check for API breakage
  - handle breakage? generate different code if using different API?
  - => written code should continue to works for all versions (generator take care of the difference)

  - generate c++ code.
  - generate documentation (rst)


C++
---

We generate reflection data.

Documentation
-------------

We generate only part of the page:
 - object descriptions
 - listing: (just link to ref)
   - methods
   - events
   - object metadata

Signatures can be generated for multiple languages. (python, c++, java, .NET, matlab, ..)

We generate a fragment of page that should be included in a user written page. This is idea is to split generated code and hand written code for easier maintainance.




