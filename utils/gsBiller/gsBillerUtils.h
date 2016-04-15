#ifndef GsBillerUtils
#define GsBillerUtils

#define	HEX_FORMAT_INHX8M	0x01
#define	HEX_FORMAT_INHX32	0x02

#define SPM_PAGESIZE            64
#define SPM_PAGESIZE_BYTES      (SPM_PAGESIZE<<1)

typedef uint32_t    address_t;
typedef uint8_t     value_t;

typedef	std::array<value_t, SPM_PAGESIZE_BYTES>
  page_t;
typedef	std::map<address_t, page_t> hexFile_t;

typedef	hexFile_t::iterator	iterator;
typedef	hexFile_t::reverse_iterator	reverse_iterator;
typedef	page_t::size_type	size_type;

//The data set that results from parsing a hex file
struct hex_data
{
  //Each line of the hex file generates a block of memory at a particular address
  value_t  _fill;	    // Value returned for unset addresses
  char	format;				//Format of the parsed file (necessary?)
  bool	segment_addr_rec;		// Uses/Has a segment address record
  bool	linear_addr_rec;		// Uses/Has a linear address record
  hexFile_t   blocks;			// List of data blocks

hex_data() : _fill(0), segment_addr_rec(false), linear_addr_rec(false) {}
hex_data(const std::string &s) : _fill(0), segment_addr_rec(false), linear_addr_rec(false)
  {
    load(s);
  }
  iterator    begin() { return blocks.begin(); }
  iterator    end() { return blocks.end(); }

  void	compact();		// Merge adjacent blocks
  void	clear();		//Delete everything
  void	erase(address_t);	// Erase a single element
  void 	erase(address_t first, address_t last);  // Erase [first, last]
  value_t  fill()  { return _fill; }
  void	    fill(value_t f)  { _fill = f; }
  size_type   size();
  size_type   size_below_addr(address_t);
  size_type   size_in_range(address_t, address_t);    //number of words in [lo, hi)
  address_t   max_addr_below(address_t);

  address_t   min_address() const;	// Lowest address
  address_t   max_address() const;	// Highest address

  bool	is_set(address_t);

  value_t& operator[](address_t);	//Array access operator
  value_t  get(address_t);		// Return the value at address
  void	set(address_t, value_t);	// Set the value at address

  void	load(const std::string&);	// Load from a file
  void	read(std::istream &);			// Read data from an input stream
  void	write(const char *);			//Save hex data to a hex file
  void	write(std::ostream &);			//Write all data to an output stream
  void	tidy(size_type length);			// Make things pretty
};

bool compare(hex_data&, hex_data&, value_t, address_t, address_t);

#endif
