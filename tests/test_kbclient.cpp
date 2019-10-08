//
// Author: Jun Zhu, zhujun981661@gmail.com
//
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "karabo-bridge/kb_client.hpp"


namespace karabo_bridge {

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;

template<typename T>
MsgpackObject packObject(T x)
{
  msgpack::sbuffer sbuf;
  msgpack::pack(sbuf, x);
  msgpack::object_handle oh = msgpack::unpack(sbuf.data(), sbuf.size());
  return oh.get().as<MsgpackObject>();
}

MsgpackObject packBin(std::vector<int> x)
{
  msgpack::sbuffer sbuf;
  msgpack::pack(sbuf, msgpack::type::raw_ref(reinterpret_cast<const char *>(x.data()),
                                             x.size() * sizeof(int)));
  msgpack::object_handle oh = msgpack::unpack(sbuf.data(), sbuf.size());
  return oh.get().as<MsgpackObject>();
}

TEST(TestMultipartMsg, TestGeneral)
{
  MultipartMsg mpmsg;

  std::stringstream ss;
  msgpack::pack(ss, "European XFEL");
  mpmsg.emplace_back(zmq::message_t(ss.str().data(), ss.str().size()));

  ss.str("");
  msgpack::pack(ss, "is");
  mpmsg.emplace_back(zmq::message_t(ss.str().data(), ss.str().size()));

  ss.str("");
  msgpack::pack(ss, "magnificent!");
  mpmsg.emplace_back(zmq::message_t(ss.str().data(), ss.str().size()));

  EXPECT_EQ("\"European XFEL\"\n\"is\"\n\"magnificent!\"\n",
            parseMultipartMsg(mpmsg, false));
}

TEST(TestKbData, TestGeneral)
{
  auto obj1 = packObject<int>(100);
  auto obj2 = packObject<float>(0.002);
  ObjectMap obj_data{{"obj1", obj1}, {"obj2", obj2}};

  uint16_t a[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  NDArray arr1((void *) a, std::vector<std::size_t>{2, 2, 3}, "uint16_t");

  kb_data data;

  // copy from obj_data
  for (auto &v : obj_data)
  { data.insert(v); }
  EXPECT_EQ(100, obj_data["obj1"].as<int>());

  // check iterator
  auto it = data.begin();
  EXPECT_EQ("obj1", it->first);
  EXPECT_EQ(100, it->second.as<uint64_t>());
  ++it;
  EXPECT_EQ("obj2", it->first);
  EXPECT_TRUE(it->second.as<float>() - 0.002 < 1e-10);
  ++it;
  EXPECT_EQ(data.end(), it);
}

TEST(TestNdarray, TestGeneral)
{
  uint16_t a[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

  NDArray array_uint16((void *) a, std::vector<std::size_t>{2, 2, 3}, "uint16_t");
  EXPECT_EQ("uint16_t", array_uint16.dtype());
  assert(array_uint16.shape() == std::vector<std::size_t>({2, 2, 3}));
  EXPECT_EQ(12, array_uint16.size());
  EXPECT_EQ("array-like", array_uint16.containerType());
  EXPECT_TRUE(*static_cast<uint16_t *>(array_uint16.data()) - 1 < 1e-10);
  EXPECT_TRUE(*array_uint16.data<uint16_t>() - 1 < 1e-10);
  EXPECT_THAT(array_uint16.as<std::vector<uint16_t>>(),
              ElementsAreArray({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}));
  EXPECT_THAT(array_uint16.as<std::deque<uint16_t>>(),
              ElementsAreArray({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}));
  EXPECT_THAT((array_uint16.as<std::array<uint16_t, 12>>()),
              ElementsAreArray({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}));

  EXPECT_THROW(array_uint16.as<std::vector<int>>(), TypeMismatchErrorNDArray);
  EXPECT_THROW((array_uint16.as<std::map<int, int>>()), TypeMismatchErrorNDArray);
  EXPECT_THROW(array_uint16.data<int>(), TypeMismatchErrorNDArray);
  EXPECT_THROW((array_uint16.as<std::array<uint16_t, 13>>()), CastErrorNDArray);
}

TEST(TestMsgpackObject, TestGeneral)
{
  auto obj_uint = packObject<std::size_t>(2147483648);
  EXPECT_EQ(0, obj_uint.size());
  EXPECT_EQ("uint64_t", obj_uint.dtype());
  EXPECT_EQ("", obj_uint.containerType());
  EXPECT_EQ(2147483648, obj_uint.as<uint64_t>());
  // Msgpack does the overflow check. So we can give the type check task to msgpack safely.
  EXPECT_THROW(obj_uint.as<int>(), CastErrorMsgpackObject);

  auto obj_int = packObject<int>(-10);
  EXPECT_EQ(0, obj_int.size());
  EXPECT_EQ("int64_t", obj_int.dtype());
  EXPECT_TRUE(obj_int.containerType().empty());
  EXPECT_EQ(-10, obj_int.as<int64_t>());

  auto obj_float = packObject<double>(0.00222);
  EXPECT_EQ(0, obj_float.size());
  EXPECT_EQ("double", obj_float.dtype());
  EXPECT_TRUE(obj_float.containerType().empty());
  EXPECT_TRUE(obj_float.as<double>() - 0.00222 < 1e-10);
  EXPECT_THROW(obj_float.as<int>(), CastErrorMsgpackObject);

  auto obj_nil = packObject(msgpack::type::nil_t());
  EXPECT_EQ(0, obj_nil.size());
  EXPECT_EQ("MSGPACK_OBJECT_NIL", obj_nil.dtype());
  EXPECT_TRUE(obj_nil.containerType().empty());
  EXPECT_THROW(obj_nil.as<int>(), CastErrorMsgpackObject);

  std::vector<int> vec({1, 2, 3, 4});
  auto obj_arr = packObject(vec);
  EXPECT_EQ(4, obj_arr.size());
  EXPECT_EQ("uint64_t", obj_arr.dtype());
  EXPECT_EQ("array-like", obj_arr.containerType());
  EXPECT_THAT(obj_arr.as<std::vector<int>>(), ElementsAreArray(vec));
  EXPECT_THAT(obj_arr.as<std::deque<int>>(), ElementsAreArray(vec));
  EXPECT_THAT((obj_arr.as<std::array<int, 4>>()), ElementsAreArray(vec));
  EXPECT_THROW(obj_arr.as<double>(), CastErrorMsgpackObject);

  auto obj_map = packObject<std::map<int, int>>({{1, 2}, {3, 4}});
  EXPECT_EQ(2, obj_map.size());
  EXPECT_EQ("undefined", obj_map.dtype());
  EXPECT_EQ("map", obj_map.containerType());

  auto obj_bin = packBin(std::vector<int>({1, 2, 3, 4}));
  EXPECT_EQ(16, obj_bin.size());
  EXPECT_EQ("char", obj_bin.dtype());
  EXPECT_EQ("array-like", obj_bin.containerType());
  EXPECT_THROW(obj_bin.as<std::vector<float>>(), CastErrorMsgpackObject);
  EXPECT_NO_THROW(obj_bin.as<std::vector<unsigned char>>());

}

} // karabo_bridge