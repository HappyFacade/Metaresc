
#include "separate_employee.h"

#undef MR_MODE
#define MR_MODE DESC
#include "separate_employee_decl.h"

employee_t create_employee (
  char const * firstname,
  char const * lastname,
  int salary)
{
  employee_t employee;

  employee.firstname = firstname;
  employee.lastname  = lastname;
  employee.salary    = salary;

  return (employee);
}
