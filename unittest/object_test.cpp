#include "ObjectV2.h"
#include "gtest/gtest.h"

TEST(Object, init) {
	ObjectV2 obj(0, 0, 123);
	ObjectV2 obj2(0, 0, 222);
	obj.Assign(obj2);
	ASSERT_EQ(obj.RValue(), 222);
}

TEST(Object, reference) {
	ObjectV2 obj(0, 0, 100);
	ObjectV2 r = obj.LValueRef();
	
	ObjectV2 a(0, 0, 200);
	r.Assign(a);
	ASSERT_EQ(obj.RValue(), 200);
	ASSERT_EQ(obj.RValue(), 200);
	ASSERT_EQ(r.RValue(), 200);
	ASSERT_EQ(r.RValue(), 200);
	r.Assign(r.Add(a));
	ASSERT_EQ(r.RValue(), 400);
	ASSERT_EQ(obj.RValue(), 400);
	obj.Assign(ObjectV2(0, 0, 99));
	ASSERT_EQ(r.RValue(), 99);
	ASSERT_EQ(obj.RValue(), 99);
}