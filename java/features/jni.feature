Feature: JNI
  In order to access P-Store with Java
  As an user
  I want to examine the output of Java example program

  Scenario: Write and read database
  Given a Java example program
  When I run "java -Djava.library.path=. -cp . pstore.examples.PStore" command
  Then P-Store should output the example data
