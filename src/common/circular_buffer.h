
class CircularBuffer {
public:

	CircularBuffer(size_t capacity);
	~CircularBuffer();

	size_t size() const {
		return size_;
	}
	size_t capacity() const {
		return capacity_;
	}
	size_t available() const {
		return capacity_ - size_;
	}
	// Return number of bytes written.
	size_t write(const char *data, size_t bytes);
	// Return number of bytes read.
	size_t read(char *data, size_t bytes);

	size_t peek(char *data, size_t bytes);
	void reset_peek();

private:
	size_t beg_index_, end_index_, size_, capacity_, peek_index_;
	char *data_;
};
