#include "VariableEqualities.h"

//------------------------------------------------------------------------------
// Adds an equality between x and y. Returns true if an equality was added,
// false if an equality already existed beforehand.
//------------------------------------------------------------------------------
bool VariableEqualities::add(const std::size_t x, const std::size_t y)
{
  // *
  // * Return false if an equality between x and y already exists
  // *
  if (exists(x, y))
    return false;

  std::size_t set_x;
  std::size_t set_y;
  bool found_x = false;
  bool found_y = false;

  // *
  // * Find the sets that x and y are in
  // *
  for (std::size_t j = 0; j < sets.size(); ++j)
  {
    if (sets[j].find(x) != std::end(sets[j]))
    {
      set_x = j;
      found_x = true;
      break;
    }
  }
  for (std::size_t j = 0; j < sets.size(); ++j)
  {
    if (sets[j].find(y) != std::end(sets[j]))
    {
      set_y = j;
      found_y = false;
      break;
    }
  }

  // *
  // * Add the equality
  // *
  if (!found_x && !found_y) // neither is in a set
  {
    std::set<std::size_t> temp;
    temp.insert(x);
    temp.insert(y);
    sets.push_back(temp);
  }
  else if (found_x && !found_y) // x is in a set, but not y
  {
    sets[set_x].insert(y);
  }
  else if (!found_x && found_y) // y is in a set, but not x
  {
    sets[set_y].insert(x);
  }
  else // they're both in a different set
  {
    for (auto j = std::begin(sets[set_y]); j != std::end(sets[set_y]); ++j)
      sets[set_x].insert(*j);
    sets.erase(std::begin(sets) + set_y);
  }

  return true;
}


//------------------------------------------------------------------------------
// Removes all equalities
//------------------------------------------------------------------------------
void VariableEqualities::clear()
{
  sets.clear();
}


//------------------------------------------------------------------------------
// Returns true if there are no equalities
//------------------------------------------------------------------------------
bool VariableEqualities::empty() const
{
  return sets.empty();
}


//------------------------------------------------------------------------------
// Returns true if an equality exists with x
//------------------------------------------------------------------------------
bool VariableEqualities::exists(const std::size_t x) const
{
  for (std::size_t i = 0; i < sets.size(); ++i)
  {
    const auto search_x = sets[i].find(x); // find x
    if (search_x != std::end(sets[i])) // if x was found
      return true;
  }
  return false;
}


//------------------------------------------------------------------------------
// Returns true if an equality exists between x and y, false otherwise.
//------------------------------------------------------------------------------
bool VariableEqualities::exists(const std::size_t x, const std::size_t y) const
{
  for (std::size_t i = 0; i < sets.size(); ++i)
  {
    const auto search_x = sets[i].find(x); // find x
    if (search_x != std::end(sets[i])) // if x was found, find y
    {
      const auto search_y = sets[i].find(y);
      if (search_y != std::end(sets[i])) // if y was also found, equality exists
        return true;
      else
        return false;
    }
  }
  return false;
}


//------------------------------------------------------------------------------
// Returns the VariableEqualities as a 2d vector.
// Each row in the vector, all individuals are equal
//------------------------------------------------------------------------------
std::vector<std::vector<std::size_t> > VariableEqualities::get2dVector() const
{
  std::vector<std::vector<std::size_t> > vec;
  for (std::size_t i = 0; i < sets.size(); ++i)
  {
    std::vector<std::size_t> row;
    for (auto it = std::begin(sets[i]); it != std::end(sets[i]); ++it)
      row.push_back(*it);
    vec.push_back(row);
  }
  return vec;
}


//------------------------------------------------------------------------------
// Returns a string of the contents of this data strucutre. Any variables that
// are equal to each other are written on the same line.
//------------------------------------------------------------------------------
std::string VariableEqualities::getEqualitiesString() const
{
  std::ostringstream oss;
  for (std::size_t i = 0; i < sets.size(); ++i)
  {
    for (auto it = std::begin(sets[i]); it != std::end(sets[i]); ++it)
      oss << *it << " ";
    if (i != sets.size() - 1)
      oss << "\n";
  }
  return oss.str();
}


//------------------------------------------------------------------------------
// Takes another VariableEqualities object and merges those equalities into this
// object.
//------------------------------------------------------------------------------
void VariableEqualities::merge(const VariableEqualities &other)
{
  const std::vector<std::vector<std::size_t> > otherVec = other.get2dVector();
  for (std::size_t i = 0; i < otherVec.size(); ++i)
  {
    for (std::size_t j = 1; j < otherVec[i].size(); ++j)
      add(otherVec[i][0], otherVec[i][j]);
  }
}

