*gallery* series 1.12
=====================


There are no new gallery-specific features in this release relative to the previous series.




.. Optional description of series


.. New features

Other
-----

Platform/compiler support
~~~~~~~~~~~~~~~~~~~~~~~~~

This series is the first to support macOS Mojave. As of gallery 1.12.00, gallery will no longer support macOS Sierra. 
In addition, the following compilers have been introduced for gallery 1.12.00:

* GCC 8.2.0 with C++17 enabled (e19 qualifier)
* Clang 7.0.0 with C++17 enabled (c2 qualifier)

See `here <https://cdcvs.fnal.gov/redmine/projects/cet-is-public/wiki/AboutQualifiers#Primary-qualifiers>`_ for more detailed information about primary qualifiers.


Python 3 support
~~~~~~~~~~~~~~~~

This series is the first to support Python 3, allowing users roughly 1 year to switch to Python 3 before the Python 2's end-of-life of January 1, 2020. 
Note that for technical reasons, Python 3 builds are not available for SLF6 platforms. 
In addition, the fhicl Python extension module is only supported for version 1.12.03 and newer.


.. Other

Breaking changes
----------------

All uses of boost::any have been replaced with std::any. 
Any users that have provided fhicl::detail::decode overloads should change their function signatures to use std::any instead (with the '#include <any>' header dependency).



.. Breaking changes


.. 
    h3(#releases){background:darkorange}. %{color:white}&nbsp; _gallery_ releases%


*gallery* suite release notes 1.12.01 (2019/02/07)
==============================================


* An *gallery 1.12* `release <releaseNotes>`_
* `Download page <https://scisoft.fnal.gov/scisoft/bundles/gallery/1.12.01/gallery-1.12.01.html>`_

External package changes
------------------------

* cetbuildtools 
  
  * v7_09_02 (prev) -> v7_10_00 (this version)
  * *Build only package*

.. External package changes

Bug fixes
---------

This release uses a new version of cetbuildtools, fixing a bug that prevented building from source on macOS.

.. Bug fixes





Known issues
------------

The fhicl Python extension module is not supported for Python 3 builds--it is re-enabled for gallery suite 1.12.03.



.. Depends on

* canvas 3.07.01
* canvas_root_io 1.03.01
* cetlib 3.07.01
* cetlib_except 1.03.02
* fhicl-cpp 4.09.01
* hep_concurrency 1.03.02
* messagefacility 2.04.01


..
    ###
    ### The following are lines that should be placed in the release notes
    ### pages of individual packages.
    ###

