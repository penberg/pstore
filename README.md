# pstore

[![Build Status](https://secure.travis-ci.org/penberg/pstore.png?branch=master)](http://travis-ci.org/penberg/pstore)

Pstore is a high-performance, read-optimized database system. The core
is implemented as a shared library `libpstore` that can be embedded in
third-party applications.

There's also a CLI application `pstore` that is used to manipulate and
inspect pstore files, and convert CSV files into pstore format.

The design of pstore is loosely based on [C-Store][] which is another
Open Source read-optimized database. The main idea is to store tabular
data in column-order to improve read performance for special purpose
workloads.

[C-Store]: http://db.csail.mit.edu/projects/cstore/

## Installation

First, install dependencies:

**Fedora:**

```
$ yum install openssl-devel
```

**Debian:**

```
$ apt-get install libssl-dev
```

Then install pstore:

```
$ make install
```

The command installs an executable ``pstore`` to ``$HOME/bin``.

## Usage

The first step is to create a database file. You can import a CSV file
into a pstore database with the following command:

```
pstore import dataset.csv dataset.out
```

After that, you can use the `pstore cat` command to look into the imported
database:

```
pstore cat dataset.out
```

With `pstore import --append` you can append more data into a pstore database:

```
pstore import --append moredata.csv dataset.out
```

## Running Tests

Pstore's test suite requires [Ruby][] 1.9 and [Bundler][]. If your
operating system does not have Ruby 1.9, take a look at [rbenv][] or
[RVM][].

When you have both Ruby 1.9 and Bundler, install Rubygem dependencies:

```
bundle install
```

Then run the tests:

```
make check
```

[Bundler]: http://gembundler.com/
[Ruby]:    http://www.ruby-lang.org/en/
[RVM]:     http://beginrescueend.com/
[rbenv]:   https://github.com/sstephenson/rbenv

## License

Pstore is distributed under the LGPL version 2.1 license.
