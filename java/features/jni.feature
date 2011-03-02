Feature: JNI
  In order to access P-Store with Java
  As an user
  I want to examine the output of Java example program

  Scenario: Write and read database
  Given a Java example program
  When I run "java -Djava.library.path=. -cp . pstore.examples.PStore" command
  Then P-Store should output
    """
    table, 0
    Column1, 0, STRING
    col1.1
    col2.1
    Column2, 1, STRING
    col1.2
    col2.2
    Column3, 2, STRING
    col1.3
    col2.3

    """
