What is it?
===========

P-Store is a simple read-optimized database system. The command line tool 
`pstore` implements two commands: `import` and `cat`. The former can be used to
convert a CSV file into a P-Store database and the latter can be used to
examine contents of the database.

The design of this system is loosely based on [C-Store][1] which is another
Open Source read-optimized database. The main idea is to store tabular data in
column-order to improve read performance for special purpose workloads.

P-Store is in very early stages so don't expect a whole lot from it.

  [1]: http://db.csail.mit.edu/projects/cstore/


How can I try it out?
=====================

The first step is to create a database file. You can import a CSV file into a
P-Store database with the following command:

    $ pstore import dataset.csv dataset.out

After that, you can use the `pstore cat` command to look into the imported
database:

    $ pstore cat dataset.out

With `pstore import --append` you can append more data into a P-Store database:

    $ pstore import --append moredata.csv dataset.out


Running the Cucumber tests
==========================

You can use the following command to install Cucumber on your machine:

    $ sudo gem install cucumber rspec fastercsv

Please note that the RubyGems version in Mac OS X 10.5, for example, is too
old to install Cucumber successfully, so you need to upgrade it first. Also,
Ruby version 1.8.7 must be used as at least `fastercsv` shall no longer be
used in 1.9.x versions.

On Mac OS X, it is recommended to use Ruby Version Manager (RVM) instead of
installing Ruby with MacPorts or Homebrew:

    http://rvm.beginrescueend.com/


Java Native Interface
=====================

P-Store provides an API for the Java programming language. To build the JNI API
of the P-Store you will need to set `JAVA_HOME` enviroment variable.
For example on Mac OS X:

    export JAVA_HOME=`/usr/libexec/java_home`
