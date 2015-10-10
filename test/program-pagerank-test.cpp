#include "simit-test.h"

#include <iostream>
#include "graph.h"
#include "program.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(Program, pagerank) {
  // Pages
  Set pages;
  FieldRef<double> outlinks = pages.addField<double>("outlinks");
  FieldRef<double> pr        = pages.addField<double>("pr");

  // Links
  Set links(pages,pages);

  // Add pages
  ElementRef A = pages.add();
  ElementRef B = pages.add();
  ElementRef C = pages.add();
  ElementRef D = pages.add();
  ElementRef E = pages.add();
  
  outlinks(A) = {0};
  outlinks(B) = {0};
  outlinks(C) = {0};
  outlinks(D) = {0};
  outlinks(E) = {0};
  
  links.add(A,E);
  links.add(B,A);
  links.add(B,E);
  links.add(C,A);
  links.add(C,E);
  links.add(D,A);
  links.add(D,E);
  
  Function func = loadFunction(TEST_FILE_NAME, "main");

  func.bind("pages", &pages);
  func.bind("links", &links);

  func.init();
  func.unmapArgs();

  for (size_t i=0; i < 10; ++i) {
    func.run();
  }

  func.mapArgs();

  SIMIT_EXPECT_FLOAT_EQ(0.341250, pr.get(A));
  SIMIT_EXPECT_FLOAT_EQ(0.150000, pr.get(B));
  SIMIT_EXPECT_FLOAT_EQ(0.150000, pr.get(C));
  SIMIT_EXPECT_FLOAT_EQ(0.150000, pr.get(D));
  SIMIT_EXPECT_FLOAT_EQ(0.63131250000000005, pr.get(E));
}
