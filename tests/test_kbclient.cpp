//
// Author: Jun Zhu, zhujun981661@gmail.com
//
#include <iostream>
#include <future>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "karabo-bridge/kb_client.hpp"


namespace karabo_bridge {

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;

/*
 * helper functions for unittest
 */

template<typename T>
msgpack::object_handle _packObject_t(T x) {
  msgpack::sbuffer sbuf;
  msgpack::pack(sbuf, x);
  return msgpack::unpack(sbuf.data(), sbuf.size());
}

msgpack::object_handle _packBin_t(std::vector<int> x) {
  msgpack::sbuffer sbuf;
  msgpack::pack(sbuf, msgpack::type::raw_ref(reinterpret_cast<const char *>(x.data()),
                                             x.size() * sizeof(int)));
  return msgpack::unpack(sbuf.data(), sbuf.size());
}

/*
 * test cases
 */

TEST(TestMultipartMsg, TestGeneral) {
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

TEST(TestClient, TestTimeout) {
  // test client with short timeout
  int timeout = 100;
  Client client(timeout);
  client.connect("tcp://localhost:12345");

  auto future = std::async(std::launch::async, [&client]() {
      client.next();
  });
  EXPECT_TRUE(future.wait_for(std::chrono::milliseconds(2*timeout)) == std::future_status::ready);

  //  test client without timeout
  auto client_inf = new Client;
  client_inf->connect("tcp://localhost:12346");

  auto future_inf = std::async(std::launch::async, [&client_inf]() {
      client_inf->next();
  });
  EXPECT_TRUE(future_inf.wait_for(std::chrono::milliseconds(2*timeout)) == std::future_status::timeout);
  delete client_inf; // close the blocking socket
}

TEST(TestKbData, TestGeneral) {
  auto oh1 = _packObject_t<int>(100);
  auto oh2 = _packObject_t<float>(0.002);
  ObjectMap obj_data{{"obj1", oh1.get().as<MsgpackObject>()},
                     {"obj2", oh2.get().as<MsgpackObject>()}};

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

TEST(TestNdarray, TestGeneral) {
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

TEST(TestMsgpackObject, TestGeneral) {
  auto oh_uint = _packObject_t<std::size_t>(2147483648);
  auto obj_uint = oh_uint.get().as<MsgpackObject>();
  EXPECT_EQ(0, obj_uint.size());
  EXPECT_EQ("uint64_t", obj_uint.dtype());
  EXPECT_EQ("", obj_uint.containerType());
  EXPECT_EQ(2147483648, obj_uint.as<uint64_t>());
  // Msgpack does the overflow check. So we can give the type check task to msgpack safely.
  EXPECT_THROW(obj_uint.as<int>(), CastErrorMsgpackObject);

  auto oh_int = _packObject_t<int>(-10);
  auto obj_int = oh_int.get().as<MsgpackObject>();
  EXPECT_EQ(0, obj_int.size());
  EXPECT_EQ("int64_t", obj_int.dtype());
  EXPECT_TRUE(obj_int.containerType().empty());
  EXPECT_EQ(-10, obj_int.as<int64_t>());

  auto oh_float = _packObject_t<double>(0.00222);
  auto obj_float = oh_float.get().as<MsgpackObject>();
  EXPECT_EQ(0, obj_float.size());
  EXPECT_EQ("double", obj_float.dtype());
  EXPECT_TRUE(obj_float.containerType().empty());
  EXPECT_TRUE(obj_float.as<double>() - 0.00222 < 1e-10);
  EXPECT_THROW(obj_float.as<int>(), CastErrorMsgpackObject);

  auto oh_nil = _packObject_t(msgpack::type::nil_t());
  auto obj_nil = oh_nil.get().as<MsgpackObject>();
  EXPECT_EQ(0, obj_nil.size());
  EXPECT_EQ("MSGPACK_OBJECT_NIL", obj_nil.dtype());
  EXPECT_TRUE(obj_nil.containerType().empty());
  EXPECT_THROW(obj_nil.as<int>(), CastErrorMsgpackObject);

  std::vector<int> vec_int {1, 2, 3, 4};
  auto oh_arr_int = _packObject_t(vec_int);
  auto obj_arr_int = oh_arr_int.get().as<MsgpackObject>();
  EXPECT_EQ(4, obj_arr_int.size());
  // positive integer will be parsed as uint64_t
  EXPECT_EQ("uint64_t", obj_arr_int.dtype());
  EXPECT_EQ("array-like", obj_arr_int.containerType());
  EXPECT_THAT(obj_arr_int.as<std::vector<int>>(), ElementsAreArray(vec_int));
  EXPECT_THAT(obj_arr_int.as<std::deque<int>>(), ElementsAreArray(vec_int));
  EXPECT_THAT((obj_arr_int.as<std::array<int, 4>>()), ElementsAreArray(vec_int));
  EXPECT_THROW(obj_arr_int.as<double>(), CastErrorMsgpackObject);

  std::deque<float> deq_f {1, 2, 3, 4};
  auto oh_arr_f = _packObject_t(deq_f);
  auto obj_arr_f = oh_arr_f.get().as<MsgpackObject>();
  EXPECT_EQ(4, obj_arr_f.size());
  EXPECT_EQ("float", obj_arr_f.dtype());
  EXPECT_EQ("array-like", obj_arr_f.containerType());
  EXPECT_THAT(obj_arr_f.as<std::vector<float>>(), ElementsAreArray(deq_f));
  EXPECT_THAT(obj_arr_f.as<std::deque<float>>(), ElementsAreArray(deq_f));
  EXPECT_THAT((obj_arr_f.as<std::array<float, 4>>()), ElementsAreArray(deq_f));
  EXPECT_THROW(obj_arr_f.as<double>(), CastErrorMsgpackObject);

  auto oh_map = _packObject_t<std::map<int, int>>({{1, 2}, {3, 4}});
  auto obj_map = oh_map.get().as<MsgpackObject>();
  EXPECT_EQ(2, obj_map.size());
  EXPECT_EQ("undefined", obj_map.dtype());
  EXPECT_EQ("map", obj_map.containerType());

  auto oh_bin = _packBin_t(std::vector<int>({1, 2, 3, 4}));
  auto obj_bin = oh_bin.get().as<MsgpackObject>();
  EXPECT_EQ(16, obj_bin.size());
  EXPECT_EQ("char", obj_bin.dtype());
  EXPECT_EQ("array-like", obj_bin.containerType());
  EXPECT_THROW(obj_bin.as<std::vector<float>>(), CastErrorMsgpackObject);
  EXPECT_NO_THROW(obj_bin.as<std::vector<unsigned char>>());
}

} // karabo_bridge