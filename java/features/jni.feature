Feature: JNI
  In order to access P-Store with Java
  As an user
  I want to examine the output of Java example program

  Scenario: Write and read database
  Given a Java example program
  When I run "java -Djava.library.path=. -cp . pstore.examples.PStore" command
  Then P-Store should output
    """
    # Table: table
    # Column: Column1 (ID = 0, type = 1)
    col1.1
    col2.1
    # Column: Column2 (ID = 1, type = 1)
    col1.2
    col2.2
    # Column: Column3 (ID = 2, type = 1)
    col1.3
    col2.3

    """
