// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <stout/path.hpp>

#include <stout/os/exists.hpp>
#include <stout/os/getcwd.hpp>
#include <stout/os/int_fd.hpp>
#include <stout/os/open.hpp>
#include <stout/os/rm.hpp>
#include <stout/os/touch.hpp>

#include <stout/tests/utils.hpp>

using std::string;
using std::vector;


// Test many corner cases of Path::basename.
TEST(PathTest, Basename)
{
  const string PATH_SEPARATOR = stringify(os::PATH_SEPARATOR);

  // Empty path check.
  EXPECT_EQ(".", Path("").basename());

  // Check common path patterns.
  EXPECT_EQ(".", Path(".").basename());
  EXPECT_EQ("..", Path("..").basename());
  EXPECT_EQ("a", Path("a").basename());

  // Check common path patterns.
  EXPECT_EQ(PATH_SEPARATOR, Path(path::join("", "")).basename());
  EXPECT_EQ("b", Path(path::join("a", "b")).basename());
  EXPECT_EQ("c", Path(path::join("a", "b", "c")).basename());

  // Check leading slashes get cleaned up properly.
  EXPECT_EQ("a", Path(path::join("", "a")).basename());
  EXPECT_EQ("a", Path(path::join("", "a")).basename());
  EXPECT_EQ("a", Path(path::join("", "a", "")).basename());
  EXPECT_EQ("c", Path(path::join("", "a", "b", "c")).basename());
  EXPECT_EQ("b", Path(path::join("", "a", "b")).basename());
  EXPECT_EQ("b", Path(path::join("", "", "a", "", "b")).basename());

  // Check trailing slashes get cleaned up properly.
  EXPECT_EQ("a", Path(path::join("a", "")).basename());
  EXPECT_EQ("c", Path(path::join("", "a", "b", "c", "", "")).basename());
  EXPECT_EQ("c", Path(path::join("", "a", "b", "c", "", "", "")).basename());
  EXPECT_EQ(PATH_SEPARATOR, Path(path::join("", "", "")).basename());
  EXPECT_EQ(PATH_SEPARATOR, Path(path::join("", "", "", "")).basename());
}



// Test many corner cases of Path::dirname.
TEST(PathTest, Dirname)
{
  const string sep = stringify(os::PATH_SEPARATOR);
  
  // Empty path check.
  EXPECT_EQ(".", Path("").dirname());

  // Check common path patterns.
  EXPECT_EQ(".", Path(".").dirname());
  EXPECT_EQ(".", Path("..").dirname());
  EXPECT_EQ(".", Path("a").dirname());

  // Check common path patterns.
  EXPECT_EQ(sep, Path(sep).dirname());
  EXPECT_EQ(".", Path(".").dirname());
  EXPECT_EQ(".", Path("..").dirname());

  EXPECT_EQ(".", Path("a").dirname());
  EXPECT_EQ("a", Path(path::join("a", "b")).dirname());
  EXPECT_EQ(path::join("a", "b"), Path(path::join("a", "b", "c")).dirname());

  // Check leading slashes get cleaned up properly.
  EXPECT_EQ(sep, Path(path::join("", "a")).dirname());
  EXPECT_EQ(sep, Path(path::join(sep, "a")).dirname());
  EXPECT_EQ(sep, Path(path::join("", "a", "")).dirname());
  EXPECT_EQ(path::join("", "a"), Path(path::join("", "a", "b")).dirname());
  EXPECT_EQ(path::join(sep, "a"), Path(path::join(sep, "a", "", "b")).dirname());
  EXPECT_EQ(path::join(sep, "a", "b"), Path(path::join("", "a", "b", "c")).dirname());
  // Check intermittent slashes get handled just like ::dirname does.
  EXPECT_EQ(path::join("", "a", "", "b"), Path(path::join("", "a", "", "b", "", "c", sep)).dirname());
  EXPECT_EQ(path::join(sep, "a", "b"), Path(path::join(sep, "a", "b", "", "c")).dirname());

  // Check trailing slashes get cleaned up properly.
  EXPECT_EQ(".", Path(path::join("a", "")).dirname());
  EXPECT_EQ(path::join("a", "b"), Path(path::join("a", "b", "c")).dirname());
  EXPECT_EQ(path::join("", "a", "b"), Path(path::join("", "a", "b", "c", "")).dirname());
  EXPECT_EQ(path::join("", "a", "b"), Path(path::join("", "a", "b", "c", sep)).dirname());
  EXPECT_EQ(path::join("", "a", "b"), Path(path::join("", "a", "b", "c", sep, "")).dirname());
  EXPECT_EQ(sep, Path(path::join("", sep)).dirname());
  EXPECT_EQ(sep, Path(path::join("", sep, "")).dirname());
}


TEST(PathTest, Extension)
{
  const string sep = stringify(os::PATH_SEPARATOR);
  EXPECT_NONE(Path(".").extension());
  EXPECT_NONE(Path("..").extension());
  EXPECT_NONE(Path("a").extension());

  EXPECT_NONE(Path(path::join("", "a")).extension());
  EXPECT_NONE(Path(sep).extension());

  EXPECT_NONE(Path(path::join("", "a.b", "c")).extension());

  EXPECT_SOME_EQ(".txt", Path("a.txt").extension());
  EXPECT_SOME_EQ(".txt", Path(path::join("", "a", "b.txt")).extension());
  EXPECT_SOME_EQ(".txt", Path(path::join("", "a.b", "c.txt")).extension());

  EXPECT_SOME_EQ(".gz", Path("a.tar.gz").extension());
  EXPECT_SOME_EQ(".gz", Path(path::join("", "a.tar.gz")).extension());

  EXPECT_SOME_EQ(".bashrc", Path(".bashrc").extension());
  EXPECT_SOME_EQ(".bashrc", Path(path::join("", ".bashrc")).extension());
}


TEST(PathTest, Join)
{
  const string sep = stringify(os::PATH_SEPARATOR);

  EXPECT_EQ("a" + sep + "b" + sep + "c", path::join("a", "b", "c"));
  EXPECT_EQ(sep + "a" + sep + "b" + sep + "c", path::join(sep + "a", "b", "c"));

  EXPECT_EQ("", path::join(vector<string>()));
  EXPECT_EQ("a" + sep + "b" + sep + "c", path::join(vector<string>({"a", "b", "c"})));

  // TODO(cmaloney): This should join to ""
  EXPECT_EQ(sep, path::join(vector<string>({"", "", ""})));

  // Interesting corner cases around being the first, middle, last.
  EXPECT_EQ(sep + "asdf", path::join(sep, "asdf"));
  EXPECT_EQ(sep, path::join("", sep, ""));
  EXPECT_EQ("ab" + sep, path::join("ab" + sep, "", sep));
  EXPECT_EQ(sep + "ab", path::join(sep, sep, "ab"));
  EXPECT_EQ("ab" + sep, path::join("ab", sep, sep));
  EXPECT_EQ(sep + "ab", path::join(sep, "", sep + "ab"));

  // Check trailing and leading slashes get cleaned up.
  EXPECT_EQ("a" + sep + "b" + sep + "c" + sep, path::join("a" + sep, "b" + sep, "c" + sep));
  EXPECT_EQ(sep + "a" + sep + "b" + sep + "c", path::join(sep + "a", sep + "b", sep + "c"));
  EXPECT_EQ(sep + "a" + sep + "b" + sep + "c" + sep, path::join(sep + "a" + sep, sep + "b" + sep, sep + "c" + sep));
  EXPECT_EQ("a" + sep + "b" + sep + "c" + sep, path::join("a" + sep, sep + "b" + sep, sep + "c" + sep));
}


TEST(PathTest, Absolute)
{
#ifdef __WINDOWS__
  // Check absolute paths.
  EXPECT_TRUE(path::absolute("C:\\foo\\bar\\baz"));
  EXPECT_TRUE(path::absolute("c:\\"));
  EXPECT_TRUE(path::absolute("C:/"));
  EXPECT_TRUE(path::absolute("c:/"));
  EXPECT_TRUE(path::absolute("X:\\foo"));
  EXPECT_TRUE(path::absolute("X:\\foo"));
  EXPECT_TRUE(path::absolute("y:\\bar"));
  EXPECT_TRUE(path::absolute("y:/bar"));
  EXPECT_TRUE(path::absolute("\\\\?\\"));
  EXPECT_TRUE(path::absolute("\\\\?\\C:\\Program Files"));
  EXPECT_TRUE(path::absolute("\\\\?\\C:/Program Files"));
  EXPECT_TRUE(path::absolute("\\\\?\\C:\\Path"));
  EXPECT_TRUE(path::absolute("\\\\server\\share"));

  // Check invalid paths.
  EXPECT_FALSE(path::absolute("abc:/"));
  EXPECT_FALSE(path::absolute("1:/"));
  EXPECT_TRUE(path::absolute("\\\\?\\relative"));

  // Check relative paths.
  EXPECT_FALSE(path::absolute("relative"));
  EXPECT_FALSE(path::absolute("\\file-without-disk"));
  EXPECT_FALSE(path::absolute("/file-without-disk"));
  EXPECT_FALSE(path::absolute("N:file-without-dir"));
#else
  // Check absolute paths.
  EXPECT_TRUE(path::absolute("/"));
  EXPECT_TRUE(path::absolute("/foo"));
  EXPECT_TRUE(path::absolute("/foo/bar"));
  EXPECT_TRUE(path::absolute("/foo/bar/../baz"));

  // Check relative paths.
  EXPECT_FALSE(path::absolute(""));
  EXPECT_FALSE(path::absolute("."));
  EXPECT_FALSE(path::absolute(".."));
  EXPECT_FALSE(path::absolute("../"));
  EXPECT_FALSE(path::absolute("./foo"));
  EXPECT_FALSE(path::absolute("../foo"));
#endif // __WINDOWS__
}


TEST(PathTest, Comparison)
{
  EXPECT_TRUE(Path("a") == Path("a"));
  EXPECT_FALSE(Path("a") == Path("b"));

  EXPECT_TRUE(Path("a") != Path("b"));
  EXPECT_FALSE(Path("a") != Path("a"));

  EXPECT_TRUE(Path("a") < Path("b"));
  EXPECT_FALSE(Path("b") < Path("a"));

  EXPECT_TRUE(Path("a") <= Path("b"));
  EXPECT_TRUE(Path("a") <= Path("a"));
  EXPECT_FALSE(Path("b") <= Path("a"));

  EXPECT_TRUE(Path("b") > Path("a"));
  EXPECT_FALSE(Path("a") > Path("a"));

  EXPECT_TRUE(Path("b") >= Path("a"));
  EXPECT_TRUE(Path("b") >= Path("b"));
  EXPECT_FALSE(Path("a") >= Path("b"));
}


class PathFileTest : public TemporaryDirectoryTest {};


TEST_F(PathFileTest, ImplicitConversion)
{
  // Should be implicitly converted to string for the various os::_ calls.
  const Path testfile(path::join(os::getcwd(), "file.txt"));

  // Create the test file.
  ASSERT_SOME(os::touch(testfile));
  ASSERT_TRUE(os::exists(testfile));

  // Open and close the file.
  Try<int_fd> fd = os::open(
      testfile,
      O_RDONLY,
      S_IRUSR | S_IRGRP | S_IROTH);
  ASSERT_SOME(fd);
  close(fd.get());

  // Delete the file.
  EXPECT_SOME(os::rm(testfile));
}
