#include "fixed_istream_buffer.h"
#include "microtest.h" // do not use function calls in test asserts if not idempotent
#include <sstream>
#include <iostream>

const std::string in("0123456789ABCD");

TEST(can_offset_iterate_stream) {
   std::stringstream stream(in);
   fixed_istream_buffer buf(stream, 4);
   for(int i = 0; i < in.size(); ++i) {
     ASSERT_EQ(static_cast<char>(buf.underflow()), in.at(i));
     buf.seekoff(1, std::ios_base::cur, std::ios_base::in);
   }
}

TEST(gets_position_offset_iterate_stream) {
   std::stringstream stream(in);
   fixed_istream_buffer buf(stream, 4);
   for(int i = 0; i < in.size() - 1; ++i) {
     unsigned actual = buf.seekoff(1, std::ios_base::cur, std::ios_base::in);
     ASSERT_EQ(actual, i + 1);
   }
     unsigned actual = buf.seekoff(1, std::ios_base::cur, std::ios_base::in);
     ASSERT_EQ(actual, -1);
}

TEST(gets_position_offset_begin_iterate_stream) {
   std::stringstream stream(in);
   fixed_istream_buffer buf(stream, 4);
   for(int i = 0; i < in.size(); ++i) {
     unsigned actual = buf.seekoff(i, std::ios_base::beg, std::ios_base::in);
     ASSERT_EQ(actual, i);
   }
}

TEST(gets_position_pos_iterate_stream) {
   std::stringstream stream(in);
   fixed_istream_buffer buf(stream, 4);
   for(int i = 0; i < in.size(); ++i) {
     unsigned actual = buf.seekpos(i, std::ios_base::in);
     ASSERT_EQ(actual, i);
   }
}

TEST(can_position_iterate_stream) {
   std::stringstream stream(in);
   fixed_istream_buffer buf(stream, 4);
   for(int i = 0; i < in.size(); ++i) {
     buf.seekpos(i, std::ios_base::in);
     ASSERT_EQ(static_cast<char>(buf.underflow()), in.at(i));
   }
}

TEST(can_seek_pos_outside_current_buffer) {
   std::stringstream stream(in);
   fixed_istream_buffer buf(stream, 4);
   unsigned pos = 0;
   pos = buf.seekpos(7, std::ios_base::in);
   ASSERT_EQ(pos, 7);
   ASSERT_EQ(static_cast<char>(buf.underflow()), '7');
   pos = buf.seekpos(4, std::ios_base::in);
   ASSERT_EQ(pos, 4);
   ASSERT_EQ(static_cast<char>(buf.underflow()), '4');
   pos = buf.seekpos(3, std::ios_base::in);
   ASSERT_EQ(pos, -1);
}

TEST(can_seek_offset_outside_current_buffer) {
   std::stringstream stream(in);
   fixed_istream_buffer buf(stream, 4);
   unsigned pos = 0;
   pos = buf.seekoff(7, std::ios_base::cur, std::ios_base::in);
   ASSERT_EQ(pos, 7);
   ASSERT_EQ(static_cast<char>(buf.underflow()), '7');
   pos = buf.seekoff(-3, std::ios_base::cur, std::ios_base::in);
   ASSERT_EQ(pos, 4);
   ASSERT_EQ(static_cast<char>(buf.underflow()), '4');
   pos = buf.seekoff(-1, std::ios_base::cur, std::ios_base::in);
   ASSERT_EQ(pos, -1);
}

TEST(can_seek_pos_inside_current_buffer) {
   std::stringstream stream(in);
   fixed_istream_buffer buf(stream, 4);
   unsigned pos = 0;
   pos = buf.seekpos(3, std::ios_base::in);
   ASSERT_EQ(pos, 3);
   ASSERT_EQ(static_cast<char>(buf.underflow()), '3');
   pos = buf.seekpos(0, std::ios_base::in);
   ASSERT_EQ(pos, 0);
   ASSERT_EQ(static_cast<char>(buf.underflow()), '0');
}

TEST(can_seek_offset_inside_current_buffer) {
   std::stringstream stream(in);
   fixed_istream_buffer buf(stream, 4);
   unsigned pos = 0;
   pos = buf.seekoff(3, std::ios_base::cur, std::ios_base::in);
   ASSERT_EQ(pos, 3);
   ASSERT_EQ(static_cast<char>(buf.underflow()), '3');
   pos = buf.seekoff(-3, std::ios_base::cur, std::ios_base::in);
   ASSERT_EQ(pos, 0);
   ASSERT_EQ(static_cast<char>(buf.underflow()), '0');
}

TEST(can_seek_partially_outside_current_buffer) {
   std::stringstream stream(in);
   fixed_istream_buffer buf(stream, 4);
   unsigned pos = 0;
   pos = buf.seekpos(5, std::ios_base::in);
   ASSERT_EQ(pos, 5);
   ASSERT_EQ(static_cast<char>(buf.underflow()), '5');
   pos = buf.seekpos(2, std::ios_base::in);
   ASSERT_EQ(pos, 2);
   ASSERT_EQ(static_cast<char>(buf.underflow()), '2');
   pos = buf.seekpos(1, std::ios_base::in);
   ASSERT_EQ(pos, -1);
}

TEST(can_seek_first_element_across_multiple_buffers) {
   std::stringstream stream(in);
   fixed_istream_buffer buf(stream, 4);
   unsigned pos = buf.seekpos(8, std::ios_base::in);
   ASSERT_EQ(pos, 8);
   ASSERT_EQ(static_cast<char>(buf.underflow()), '8');
}

TEST(can_seek_last_element_across_multiple_buffers) {
   std::stringstream stream(in);
   fixed_istream_buffer buf(stream, 4);
   unsigned pos = buf.seekpos(11, std::ios_base::in);
   ASSERT_EQ(pos, 11);
   ASSERT_EQ(static_cast<char>(buf.underflow()), 'B');
}

TEST(can_seek_final_across_multiple_buffers) {
   std::stringstream stream(in);
   fixed_istream_buffer buf(stream, 4);
   unsigned pos = buf.seekpos(13, std::ios_base::in);
   ASSERT_EQ(pos, 13);
   ASSERT_EQ(static_cast<char>(buf.underflow()), 'D');
}

TEST(can_get_current_position) {
   std::stringstream stream(in);
   fixed_istream_buffer buf(stream, 4);
   buf.seekpos(3, std::ios_base::in);
   unsigned pos = buf.seekoff(0, std::ios_base::cur, std::ios_base::in);
   ASSERT_EQ(pos, 3);
   ASSERT_EQ(static_cast<char>(buf.underflow()), '3');
}

TEST(returns_eof_when_full_seek_past_end) {
   std::stringstream stream(in);
   fixed_istream_buffer buf(stream, 4);
   buf.seekpos(13, std::ios_base::in);
   buf.seekpos(17, std::ios_base::in);
   ASSERT_EQ(static_cast<char>(buf.underflow()), std::char_traits<char>::eof());
}

TEST(returns_eof_when_partial_seek_past_end) {
   std::stringstream stream(in);
   fixed_istream_buffer buf(stream, 4);
   buf.seekpos(15, std::ios_base::in);
   ASSERT_EQ(static_cast<char>(buf.underflow()), std::char_traits<char>::eof());
}

TEST_MAIN();
