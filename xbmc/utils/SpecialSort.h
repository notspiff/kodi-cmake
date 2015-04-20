#pragma once
/*
 *      Copyright (C) 2012-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <map>
#include <set>
#include <vector>

class SpecialSort
{
public:
  template <typename T>
  static std::vector<T> SortTopologically(const std::vector< std::pair<T, T> > &topology);

private:
  static void sortTopologically(int current, const std::vector< std::set<int> > &edges, std::vector<bool> &visited, std::vector<int> &stack);
};

template <typename T>
std::vector<T> SpecialSort::SortTopologically(const std::vector< std::pair<T, T> > &topology)
{
  typename std::map<int, T> key2ValueMapping;
  typename std::map<T, int> value2KeyMapping;
  std::vector< std::set<int> > edges;

  int key = 0;
  // map the values to comparable and usable keys and build a map with all
  // the sources and their destinations
  for (typename std::vector< std::pair<T, T> >::const_iterator topologyIt = topology.begin(); topologyIt != topology.end(); ++topologyIt)
  {
    size_t sourceKey = -1, destKey = -1;
    // handle the source item
    typename std::map<T, int>::const_iterator it = value2KeyMapping.find(topologyIt->first);
    if (it == value2KeyMapping.end())
    {
      sourceKey = key;
      value2KeyMapping.insert(make_pair(topologyIt->first, sourceKey));
      key2ValueMapping.insert(make_pair(sourceKey, topologyIt->first));
      key++;
    }
    else
      sourceKey = it->second;

    // handle the destination item
    it = value2KeyMapping.find(topologyIt->second);
    if (it == value2KeyMapping.end())
    {
      destKey = key;
      value2KeyMapping.insert(make_pair(topologyIt->second, destKey));
      key2ValueMapping.insert(make_pair(destKey, topologyIt->second));
      key++;
    }
    else
      destKey = it->second;

    // add the combination to the edges mapping
    if (edges.size() <= sourceKey)
      edges.insert(edges.end(), sourceKey + 1 - edges.size(), std::set<int>());
    edges[sourceKey].insert(destKey);
  }

  size_t size = value2KeyMapping.size();
  std::vector<int> stack; stack.reserve(size);
  if (edges.size() < size)
    edges.insert(edges.end(), size - edges.size(), std::set<int>());
  std::vector<bool> visited(size, false);

  // perform a topology sort
  for (size_t i = 0; i < size; i++)
  {
    if (!visited[i])
      sortTopologically(i, edges, visited, stack);
  }

  // translate the keys back to the real values
  typename std::vector<T> result;
  for (std::vector<int>::const_iterator keyIt = stack.begin(); keyIt != stack.end(); ++keyIt)
    result.push_back(key2ValueMapping[*keyIt]);

  return result;
}

void SpecialSort::sortTopologically(int current, const std::vector< std::set<int> > &edges, std::vector<bool> &visited, std::vector<int> &stack)
{
  // mark the current node as visited
  visited[current] = true;

  for (std::set<int>::const_iterator i = edges.at(current).begin(); i != edges.at(current).end(); ++i)
  {
    if (!visited[*i])
      sortTopologically(*i, edges, visited, stack);
  }

  stack.push_back(current);
}
