#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include <set>
using namespace std;
#include <SFML/Graphics/Image.hpp>
using namespace sf;

wstring toBinary(size_t val, size_t len)
{
	wstring res;
	while (val != 0)
	{
		res.push_back(val % 2 + '0');
		val /= 2;
	}
	while (res.size() < len)
		res.push_back('0');
	reverse(res.begin(), res.end());
	return res;
}
bool doubleEqual(double a, double b)
{
	return abs(a - b) < FLT_EPSILON;
}

size_t messageSize;
size_t symbolsCount;
size_t evenCodeLen;
struct Symbol
{
	size_t value;
	size_t index;
	size_t count;
	double prob;
	double entropy;
	wstring evenCode;
	wstring shannonFanoCode;
	wstring huffmanCode;
	Symbol(size_t val, size_t index, size_t count) : value(val), index(index), count(count)
	{
		prob = (double)count / messageSize;
		entropy = -prob * log2(prob);
	}
};
bool indexCmp(const Symbol& a, const Symbol& b)
{
	return a.index < b.index;
}
bool probCmp(const Symbol& a, const Symbol& b)
{
	if (!doubleEqual(a.prob, b.prob))
		return a.prob < b.prob;
	return a.index < b.index;
}
vector<Symbol> symbols;
vector<wstring> shannonFanoCodes;
wstring curShannonFanoCode;
void shannonFanoRec(size_t l, size_t r)
{
	if (l == r)
	{
		shannonFanoCodes[l] = curShannonFanoCode;
		return;
	}
	double totalProb = 0;
	for (size_t i = l; i <= r; i++)
		totalProb += symbols[i].prob;
	double leftProb = 0;
	size_t separator;
	double minDiff = DBL_MAX;
	for (size_t i = l; i <= r; i++)
	{
		leftProb += symbols[i].prob;
		double rightProb = totalProb - leftProb;
		if (abs(leftProb - rightProb) < minDiff)
		{
			minDiff = abs(leftProb - rightProb);
			separator = i;
		}
	}
	curShannonFanoCode.push_back('0');
	shannonFanoRec(l, separator);
	curShannonFanoCode.pop_back();
	curShannonFanoCode.push_back('1');
	shannonFanoRec(separator + 1, r);
	curShannonFanoCode.pop_back();
}
void shannonFano()
{
	sort(symbols.begin(), symbols.end(), probCmp);
	shannonFanoCodes.resize(symbolsCount);
	shannonFanoRec(0, symbolsCount - 1);
	for (size_t i = 0; i < symbolsCount; i++)
		symbols[i].shannonFanoCode = shannonFanoCodes[i];
	sort(symbols.begin(), symbols.end(), indexCmp);
}

struct Node
{
	size_t index;
	double weight;
	Node* l;
	Node* r;
	Node(double weight) : weight(weight), index(0), l(nullptr), r(nullptr)
	{ }
};
vector<wstring> huffmanCodes;
wstring curHuffmanCode;
void huffmanRec(Node* node)
{
	if (node->l != nullptr && node->r != nullptr)
	{
		curHuffmanCode.push_back('0');
		huffmanRec(node->l);
		curHuffmanCode.pop_back();
		curHuffmanCode.push_back('1');
		huffmanRec(node->r);
		curHuffmanCode.pop_back();
	}
	else
	{
		huffmanCodes[node->index] = curHuffmanCode;
	}
}
void huffman()
{
	auto nodePtrWeightCmp = [](Node* a, Node* b) { return a->weight < b->weight; };
	multiset<Node*, decltype(nodePtrWeightCmp)> nodes(nodePtrWeightCmp);
	for (size_t i = 0; i < symbolsCount; i++)
	{
		Node* newNode = new Node(symbols[i].prob);
		newNode->index = symbols[i].index;
		nodes.insert(newNode);
	}
	while (nodes.size() != 1)
	{
		Node* min1 = *nodes.begin();
		nodes.erase(nodes.begin());
		Node* min2 = *nodes.begin();
		nodes.erase(nodes.begin());
		Node* newNode = new Node(min1->weight + min2->weight);
		newNode->l = min1;
		newNode->r = min2;
		nodes.insert(newNode);
	}
	huffmanCodes.resize(symbolsCount);
	huffmanRec(*nodes.begin());
	for (size_t i = 0; i < symbolsCount; i++)
		symbols[i].huffmanCode = huffmanCodes[i];
}

int main()
{
	setlocale(LC_ALL, "Russian");
	FILE* filePtr;
	freopen_s(&filePtr, "output.txt", "w", stdout);
	wcout.setf(ios::fixed);
	wcout.precision(4);

	Image image;
	if (!image.loadFromFile("picture_MY.jpg"))
		throw;

	vector<size_t> message(image.getSize().x);
	messageSize = message.size();
	for (size_t i = 0; i < image.getSize().x; i++)
	{
		size_t x = image.getPixel(i, image.getSize().y / 2).r;
		message[i] = (size_t)round(x / 20.0) * 20;
	}

	map<size_t, size_t> counts;
	for (size_t i = 0; i < message.size(); i++)
		counts[message[i]]++;
	symbolsCount = counts.size();
	evenCodeLen = (size_t)ceil(log2(symbolsCount));

	size_t index = 0;
	for (auto it = counts.begin(); it != counts.end(); it++)
	{
		symbols.push_back(Symbol(it->first, index, it->second));
		index++;
	}

	for (size_t i = 0; i < symbolsCount; i++)
		symbols[i].evenCode = toBinary(symbols[i].index, evenCodeLen);
	shannonFano();
	huffman();

	wcout << L"Список символов: " << endl;
	double totalEntropy = 0;
	for (size_t i = 0; i < symbolsCount; i++)
	{
		wcout << i + 1 << L") Символ: " << symbols[i].value << L". Частота встречаемости: " << symbols[i].prob << endl;
		totalEntropy += symbols[i].entropy;
	}
	wcout << L"Количество символов алфавита: " << symbolsCount << endl;
	wcout << L"Значение энтропии: " << totalEntropy << endl;
	wcout << L"Расчетная длина двоичного кода: " << evenCodeLen << endl;
	wcout << endl;

	wcout << L"Равномерный односимвольный код: " << endl;
	for (size_t i = 0; i < symbolsCount; i++)
	{
		wcout << i + 1 << L") Символ: " << symbols[i].value << ".";
		wcout << L" Равномерный односимвольный код: " << symbols[i].evenCode << endl;
	}
	wcout << endl;

	wcout << L"Коды Шеннона-Фано: " << endl;
	for (size_t i = 0; i < symbolsCount; i++)
	{
		wcout << i + 1 << L") Символ: " << symbols[i].value << ".";
		wcout << L" Код Шеннона-Фано: " << symbols[i].shannonFanoCode << endl;
	}
	wcout << endl;

	wcout << L"Коды Хаффмана: " << endl;
	for (size_t i = 0; i < symbolsCount; i++)
	{
		wcout << i + 1 << L") Символ: " << symbols[i].value << ".";
		wcout << L" Код Хаффмана: " << symbols[i].huffmanCode << endl;
	}
	wcout << endl;

	map<size_t, size_t> indexes;
	for (size_t i = 0; i < symbolsCount; i++)
		indexes[symbols[i].value] = symbols[i].index;

	wcout << L"Сообщение в равномерном коде:" << endl;
	size_t evenLen = 0;
	for (size_t i = 0; i < messageSize; i++)
	{
		wcout << symbols[indexes[message[i]]].evenCode;
		evenLen += symbols[indexes[message[i]]].evenCode.size();
	}
	wcout << endl;
	wcout << L"Длина закодированного сообщения: " << evenLen << endl;
	double evenMidLen = (double)evenLen / messageSize;
	wcout << L"Среднее количество бит на символ: " << evenMidLen << endl;
	double evenCompressionRatio = (double)evenLen / evenLen;
	wcout << L"Коэффициент сжатия: " << evenCompressionRatio << endl;
	wcout << endl;

	wcout << L"Сообщение в коде Шеннона-Фано:" << endl;
	size_t shanFanoLen = 0;
	for (size_t i = 0; i < messageSize; i++)
	{
		wcout << symbols[indexes[message[i]]].shannonFanoCode;
		shanFanoLen += symbols[indexes[message[i]]].shannonFanoCode.size();
	}
	wcout << endl;
	wcout << L"Длина закодированного сообщения: " << shanFanoLen << endl;
	double shanFanoMidLen = (double)shanFanoLen / messageSize;
	wcout << L"Среднее количество бит на символ: " << shanFanoMidLen << endl;
	double shanFanoCompressionRatio = (double)shanFanoLen / evenLen;
	wcout << L"Коэффициент сжатия: " << shanFanoCompressionRatio << endl;
	wcout << endl;

	wcout << L"Сообщение в коде Хаффмана:" << endl;
	size_t huffmanLen = 0;
	for (size_t i = 0; i < messageSize; i++)
	{
		wcout << symbols[indexes[message[i]]].huffmanCode;
		huffmanLen += symbols[indexes[message[i]]].huffmanCode.size();
	}
	wcout << endl;
	wcout << L"Длина закодированного сообщения: " << huffmanLen << endl;
	double huffmanMidLen = (double)huffmanLen / messageSize;
	wcout << L"Среднее количество бит на символ: " << huffmanMidLen << endl;
	double huffmanCompressionRatio = (double)huffmanLen / evenLen;
	wcout << L"Коэффициент сжатия: " << huffmanCompressionRatio << endl;
	wcout << endl;

	return 0;
}