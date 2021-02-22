#include <unistd.h>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <bitset>
#include "CNecklace.h"
#include <algorithm>

using namespace std;

int id_bits = -1;
vector<int> unique_ids;
vector<string> base_representations;

string decimal_to_binary(unsigned n){
        const int size = sizeof(n) * 8;
        std::string res;
        bool s=0;
        for (int a=0;a<size;a++)
        {
                bool bit = n >> (size-1);
                if (bit)
                {
                        s=1;
                }

                if (s)
                {
                        res.push_back(bit+'0');
                }

                n <<= 1;
        }
        if( !res.size() )
        {
                res.push_back('0');
        }
        return res;
}

string manchester_encoding(string number)
{
        string result = "";

        for(int i = 0; i < number.size(); i ++)
        {
                if( number[i] == '0' )
                {
                        result += "01";
                }
                else if( number[i] == '1' )
                {
                        result += "10";
                }
                else
                {
                        throw runtime_error("Your number is not binary!");
                }
        }

        return result;
}

bool rotationally_self_asymmetric(string number)
{
        bool result = true;
        string rotated_number = number;

        int rotated_by = 0;
        while( (rotated_by < id_bits) && result)
        {
                rotate(rotated_number.begin(), rotated_number.begin() + 1, rotated_number.end());
//              cout << "\t" << number << " : " << rotated_number << endl;
                result = number != rotated_number;

                rotated_by ++;
        }

        return result;
}

bool is_duplicate(vector<string> existing_data, string new_data)
{
        bool result = false;

        int rotated_by = 0;
        while( (rotated_by < (2 * id_bits)) && (result == false) )
        {
                result = find(existing_data.begin(), existing_data.end(), new_data) != existing_data.end();
//              cout << new_data << " : " << result << endl;

                rotate(new_data.begin(), new_data.begin() + 1, new_data.end());
                rotated_by ++;
        }

        return result;
}

struct Point
{
  double x;
  double y;
};


// Display help
void display_help()
{
  std::cout << "Usage: whycon-id-gen [-v] -l\n" <<
               "       whycon-id-gen [-v] [-d <distance>] <bits>\n\n" <<
               "    -h     Help\n" <<
               "    -v     Verbose while generating canvas (default: false)\n" <<
               "    -l     Generating the original WhyCon marker\n" <<
               "    -d     Set minimal Hamming distance (default: 1)\n";
}

std::string start_svg(const std::string size)
{
  return "<svg width=\"" + size + "\" height=\"" + size + "\" viewBox=\"0 0 1800 1800\"" +
         " preserveAspectRatio=\"xMidYMid meet\" xmlns=\"http://www.w3.org/2000/svg\">" +
         "<rect x=\"0\" y=\"0\" width=\"1800\" height=\"1800\" fill=\"white\" stroke=\"none\"/>" +
         "<circle cx=\"900\" cy=\"900\" r=\"899\" fill=\"white\" stroke=\"black\"/>" +
         "<circle cx=\"900\" cy=\"900\" r=\"700\" fill=\"black\" stroke=\"none\"/>" +
         "<circle cx=\"900\" cy=\"900\" r=\"420\" fill=\"white\" stroke=\"none\"/>" +
         "<path stroke=\"black\" d=\"M900,1800 L900,1775 M900,0 L900,25 M0,900 L25,900 M1800,900 L1775,900\"/>";
}

std::string end_svg()
{
  return "<circle cx=\"900\" cy=\"900\" r=\"240\" fill=\"white\" stroke=\"none\"/></svg>";
}

std::string draw_segments(const std::vector<std::array<Point, 2>>& points)
{
  std::ostringstream oss("<path fill=\"black\" stroke=\"none\" d=\"M900,900 L", std::ostringstream::ate);
  for(auto&& arr : points)
    oss << arr[0].x << "," << arr[0].y << " " << arr[1].x << "," << arr[1].y << " 900,900 ";
  oss << "Z\"/>";
  return oss.str();
}

std::string write_text(const std::string size, const std::string txt)
{
  return "<text x=\"1600\" y=\"100\" fill=\"black\" font-size=\"" + size + "\">" + txt + "</text>";
}

// Draw the encded ID into WhyCon marker
void draw_whycode_markers(const int id, const int idx, const int teethCount, const float w, const bool verbose)
{
  // Centre of the image
  static const int xc = 900;
  static const int yc = 900;

  std::cout << "Generating WhyCode for ID " << idx << " (encoding " << id << ") => " << idx << ".svg\n";

  // Convert lowest bit shift to binary
  std::string s = std::bitset<32>(id).to_string();

  // For each encoding bit calculate the pixel positions of each segment
  double x1, y1, x2, y2;
  std::vector<std::array<Point, 2>> points(teethCount);
  for(int i = 0; i < teethCount; i++)
  {
    x1 = xc + 650 * std::cos(-w * (2 * i + (s.at(i + 32 - teethCount) - '0') * 2.0) / 180.0 * M_PI);
    y1 = yc + 650 * std::sin(-w * (2 * i + (s.at(i + 32 - teethCount) - '0') * 2.0) / 180.0 * M_PI);
    x2 = xc + 650 * std::cos(-w * (2 * i + 1) / 180.0 * M_PI);
    y2 = yc + 650 * std::sin(-w * (2 * i + 1) / 180.0 * M_PI);

    if(verbose)
      std::cout << "Drawing Segment Size: " << x1 << "," << y1 << " " << x2 << "," << y2 << std::endl;
    points[i] = std::array<Point, 2>{Point{x1, y1}, Point{x2, y2}};
  }

  // Generate SVG
  std::ofstream ofs(std::to_string(idx) + ".svg", std::ofstream::trunc);
  if(!ofs.is_open())
  {
    std::cerr << "Error opening file\n";
    return;
  }
  ofs << start_svg("200mm");
  ofs << draw_segments(points);
  ofs << write_text("44", "&#8635; " + std::to_string(idx));
  ofs << end_svg();
  ofs.close();
}

int main(int argc, char *argv[])
{
  // Test at least one arg has been passed
  if(argc == 1)
  {
    std::cerr << "Not enough arguments or options\n\n";
    display_help();
    return 1;
  }

  // Define default parameters
  int hamming = 1;
  bool legacy = false;
  bool verbose = false;

  // Process command line arguments
  opterr = 0;
  int opt;
  while((opt = getopt(argc, argv, ":hd:vl")) != -1)
  {
    switch(opt)
    {
      case 'h':
        display_help();
        return 0;
      case 'd':
        if(std::isdigit(optarg[0]))
        {
          hamming = std::atoi(optarg);
        }
        else
        {
          std::cerr << "Invalid Hamming distance\n\n";
          display_help();
          return 1;
        }
        break;
      case 'v':
        verbose = true;
        break;
      case 'l':
        legacy = true;
        break;
      case ':':
        std::cerr << "Option requires an argument\n\n";
        display_help();
        return 1;
      default:
      case '?':
        std::cerr << "Unknown option\n\n";
        display_help();
        return 1;
    }
  }

  // Test if not too many args passed
  if(optind + 1 < argc || (legacy && optind < argc))
  {
    std::cerr << "Too many arguments or options\n\n";
    display_help();
    return 1;
  }

  // Create the orig WhyCon
  if(legacy)
  {
    std::cout << "Generating original WhyCon marker => whycon.svg\n";

    // Generate SVG
    std::ofstream ofs("whycon.svg", std::ofstream::trunc);
    if(!ofs.is_open())
    {
      std::cerr << "Error opening file\n";
      return 1;
    }
    ofs << start_svg("200mm");
    ofs << end_svg();
    ofs.close();
    return 0;
  }

  // Test a non-option argument was entered
  if(optind >= argc)
  {
    std::cerr << "Number of bits required\n\n";
    display_help();
    return 1;
  }

  // Test a non-option argument is a number and >= 3
  if(!std::isdigit(argv[optind][0]) || std::atoi(argv[optind]) < 3)
  {
    std::cerr << "Invalid number of bits. Necklace code is possible with >= 3 bits\n\n";
    display_help();
    return 1;
  }

  const int teethCount = std::atoi(argv[optind]);
  CNecklace decoder = CNecklace(teethCount, hamming);

  int a[10000];
  int n = decoder.printAll(a);
  if(decoder.verifyHamming(a, teethCount, n) < hamming)
  {
    std::cerr << "Hamming distance too low!\n";
    return 1;
  }

  id_bits = atoi(argv[1]); 

  float w = 360.0/(float)teethCount/2.0;

        for(int id = 0; id < n; id ++)
        {
                string binarized_id = decimal_to_binary(id);

                // possibly add leading zeros to make all of the representations the same length
                binarized_id = string( id_bits - binarized_id.size(), '0').append( binarized_id );

//              cout << "Processing ID " << id << "->" << binarized_id << " with manchester encoding " << manchester_encoding(binarized_id) << endl;

                string manchester_representation = manchester_encoding(binarized_id);
                if( rotationally_self_asymmetric(manchester_representation) && (! is_duplicate( base_representations, manchester_representation) ) )
                {
                        base_representations.push_back( manchester_encoding(binarized_id) );
                        unique_ids.push_back(id);
                }
        }

  cout << unique_ids.size() << " unique markers in " << id_bits << "-bit family:" << endl;
  
  for( int i : unique_ids )
  {
    draw_whycode_markers(a[i], i + 1, teethCount, w, verbose);
  }
  return 0;
}

