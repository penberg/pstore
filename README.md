What is it?
===========

P-Store is a simple read-optimized database system. The command line tool 
`pstore` implements two commands: `import` and `cat`. The former can be used to
convert a CSV file into a P-Store database and the latter can be used to
examine contents of the database.

The design of this system is loosely based on [C-Store][] which is another
Open Source read-optimized database. The main idea is to store tabular data in
column-order to improve read performance for special purpose workloads.

P-Store is in very early stages so don't expect a whole lot from it.

[C-Store]: http://db.csail.mit.edu/projects/cstore/


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


Running the integration tests
=============================

P-Store's integration tests require [Ruby][] 1.9 and [Bundler][]. If your
operating system does not have Ruby 1.9, take a look at [rbenv][] or [RVM][].

When you have both Ruby 1.9 and Bundler, install P-Store's dependencies:

    $ bundle install

Then run the integration tests:

    $ make check

[Bundler]: http://gembundler.com/
[Ruby]:    http://www.ruby-lang.org/en/
[RVM]:     http://beginrescueend.com/
[rbenv]:   https://github.com/sstephenson/rbenv


Java Native Interface
=====================

P-Store provides an API for the Java programming language. To build the JNI API
of the P-Store you will need to set `JAVA_HOME` enviroment variable.
For example on Mac OS X:

    export JAVA_HOME=`/usr/libexec/java_home`
