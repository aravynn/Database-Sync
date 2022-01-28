#include "DBClean.h"

void DBClean::printErrors(std::string title, DataPair data)
{
	if (data.size() < 1) return;

	std::cout << title << " ";
	for (auto b : data) {
		// additional formatting can be handled here. 

		std::cout << b.second.num << ' ';
	}
	std::cout << '\n';
}

void DBClean::fixDate(DataPair errLine, Table table)
{
	if (errLine.size() < 1) return;

	std::string REQUEST = "";
	DataPair filter;
	
	if (table == Table::HOSES) {
		// hose data issues. 
		REQUEST = "UPDATE Hoses SET MFGDate = IIF(MFGDate < 700000, (SELECT MFGDate FROM Hoses WHERE PK = :BackPK LIMIT 1), MFGDate), "
			"EXPDate = IIF(EXPDate < 700000, (SELECT EXPDate FROM Hoses WHERE PK = :BackPK LIMIT 1), MFGDate) WHERE PK = :CurPK";
	}
	else if (table == Table::HOSETESTS) {
		// test data issues.
		REQUEST = "UPDATE HoseTests SET Date = IIF(Date < 700000, (SELECT Date FROM HoseTests WHERE PK = :BackPK LIMIT 1), Date) "
			"WHERE PK = :CurPK";	
	}
	else {
		// invalid entry
		return;
	}

	for (auto e : errLine) {
		filter.resize(2);
		filter.at(0) = { std::string(":BackPK"), (long long)(e.second.num - 1) };
		filter.at(1) = { std::string(":CurPK"), (long long)(e.second.num) };

		// return not required.
		m_db.Complex(REQUEST, filter);
	}
}

void DBClean::fixMismatch(DataPair errLine)
{
	if (errLine.size() < 1) return;

	std::string requestOwner = "UPDATE HoseTests "
		"SET OwnerPK = (SELECT H.OwnerPK FROM Hoses as H, HoseTests as T WHERE T.PK = :PK AND T.HosePK = H.PK) "
		"WHERE PK = :PK";
	std::string requestLocation = "UPDATE Hoses as H, HoseTests as T, Locations as L "
		"SET H.LocationPK = IIF(H.LocationPK NOT IN ("
			"SELECT L.PK FROM Locations AS L, HoseTests AS T, Hoses as H WHERE L.CompanyPK = H.OwnerPK AND T.HosePK = H.PK AND T.PK = :PK),"
			" (SELECT L.PK FROM Locations AS L, HoseTests AS T, Hoses as H WHERE L.CompanyPK = H.OwnerPK AND T.HosePK = H.PK AND T.PK = :PK LIMIT 1), "
			"H.LocationPK"
		") WHERE H.PK = T.HosePK AND T.PK = :PK";
	DataPair filter;
	filter.push_back({std::string(":PK"), (long long)0});
	for (auto e : errLine) {
		filter.at(0).second = e.second.num;

		// update both values
		m_db.Complex(requestOwner, filter);
		m_db.Complex(requestLocation, filter);
	}
}

void DBClean::fixTestDataLength(DataPair errLine)
{
	std::string getTestData = "SELECT * FROM TestData WHERE TestPK = :PK";
	DataPair filter;
	filter.push_back({std::string(":PK"), (long long)0});

	for (auto e : errLine) {
		filter.at(0).second = e.second.num;
		DataPair data = m_db.Complex(getTestData, filter);

		int lastIteration{ -1 }; // store the iteration while we run through. 
		
		if (data.size() < 1) {
			// this has no information. Add a single value to "cover" the test. 
			DataPair f;
			f.push_back({ std::string("TestPK"), e.second.num });
			f.push_back({ std::string("Temperature"), 0.0 });
			f.push_back({ std::string("Pressure"), 0.0 });
			f.push_back({ std::string("IntervalNumber"), (long long)0 });

			m_db.Insert("TestData", f);
		}
		else {
			// manage the data and run the new insert. 
			std::vector<dataLine> lines;
			
			for (int i{ 0 }; i < data.size(); i+=5) {
				lines.push_back(
					dataLine(
						data.at(i).second.num,
						data.at(i+1).second.num, 
						data.at(i+2).second.dbl, 
						data.at(i+3).second.dbl, 
						data.at(i+4).second.num
						)
				);
			}
			
			std::sort(lines.begin(), lines.end(), [](dataLine a, dataLine b) {
				return a.Interval > b.Interval;
			});

			for(auto &l : lines){
				// check lines for missing or duplicate number. 
				if (lastIteration == l.Interval) {
					// duplicate, remove. 
					DataPair f;
					f.push_back({ std::string("PK"), (long long)l.PK });
					
					DataPair u;
					u.push_back({ std::string("TestPK"), (long long)-1 });

					m_db.Update("TestData", u, f);
				}
				else if (lastIteration == l.Interval + 2) {
					// missing, create.
					DataPair f;
					f.push_back({ std::string("TestPK"), (long long)l.TestPK });
					f.push_back({ std::string("Temperature"), l.Temperature });
					f.push_back({ std::string("Pressure"), l.Pressure });
					f.push_back({ std::string("IntervalNumber"), (long long)l.Interval - 1 });

					m_db.Insert("TestData", f);
				}
				lastIteration = l.Interval;
			}
			
		}
	}

}

DBClean::DBClean()
{
	std::cout << "File Error Audit \n";
	std::cout << "----------------\n";

	DataPair filter;
	filter.push_back({ std::string(), std::string() });
	bool missingLink = false;

	std::string REQUEST = "SELECT DISTINCT C.PK FROM Companies AS C WHERE C.PK NOT IN (SELECT CompanyPK FROM Contacts) OR C.PK NOT IN (SELECT CompanyPK FROM Locations)";
	DataPair BadRetVals = m_db.Complex(REQUEST, filter);
	printErrors("Companies with Link Issues:", BadRetVals);
	missingLink = (BadRetVals.size() > 0 ? true : missingLink);

	REQUEST = "SELECT DISTINCT H.PK FROM Hoses as H WHERE H.TemplatePK NOT IN (SELECT PK FROM HoseTemplates) OR H.OwnerPK NOT IN (SELECT PK FROM Companies) OR "
		"H.locationPK NOT IN (SELECT PK FROM Locations) OR H.CouplingAPK NOT IN (SELECT PK FROM FittingTemplates) OR H.CouplingBPK NOT IN (SELECT PK FROM FittingTemplates)";
	BadRetVals = m_db.Complex(REQUEST, filter);
	printErrors("Hoses with Link Issues:", BadRetVals);
	missingLink = (BadRetVals.size() > 0 ? true : missingLink);


	REQUEST = "SELECT DISTINCT H.PK FROM HoseTemplates as H WHERE H.CouplingAPK NOT IN (SELECT PK FROM FittingTemplates) OR H.CouplingBPK NOT IN (SELECT PK FROM FittingTemplates)";
	BadRetVals = m_db.Complex(REQUEST, filter);
	printErrors("Templates with Link Issues:", BadRetVals);
	missingLink = (BadRetVals.size() > 0 ? true : missingLink);


	REQUEST = "SELECT DISTINCT H.PK FROM HoseTests as H WHERE H.PK NOT IN (SELECT DISTINCT TestPK FROM TestData) OR H.HosePK NOT IN (SELECT PK FROM Hoses) OR "
		"H.OwnerPK NOT IN (SELECT PK FROM Companies)";
	BadRetVals = m_db.Complex(REQUEST, filter);
	printErrors("Hose Tests with Link Issues:", BadRetVals);
	missingLink = (BadRetVals.size() > 0 ? true : missingLink);

	if (missingLink) { 
		
		std::cout << "Fix all link errors before proceeding with other errors. \n";
		return; 
	}

	// From here one we're not dealing with links. 
	REQUEST = "SELECT PK FROM HoseTests WHERE PK NOT IN (SELECT DISTINCT T.PK FROM HoseTests as T, Companies as C, Hoses as H, Locations as L WHERE "
		"T.OwnerPK = H.OwnerPK AND L.CompanyPK = T.OwnerPK AND H.OwnerPK =  L.CompanyPK AND T.HosePK = H.PK AND H.LocationPK = L.PK)";
	BadRetVals = m_db.Complex(REQUEST, filter);
	printErrors("Data Mismatch Issues By Hose Test:", BadRetVals);
	fixMismatch(BadRetVals);

	REQUEST = "select DISTINCT T.PK from HoseTests as T, (select TestPK as pk, max(IntervalNumber) as m, count(pk) as c from TestData group by TestPK) as V WHERE "
		"T.PK = V.pk AND v.m != V.c -1 OR T.PK NOT IN (SELECT DISTINCT TestPK FROM TestData)";
	BadRetVals = m_db.Complex(REQUEST, filter);
	printErrors("Test Data With Too Much Or Little Testing Data:", BadRetVals);
	fixTestDataLength(BadRetVals);

	REQUEST = "SELECT PK FROM Hoses WHERE MFGDate < 700000 OR EXPDate < 700000";
	BadRetVals = m_db.Complex(REQUEST, filter);
	printErrors("Hoses With Bad Date Data:", BadRetVals);
	fixDate(BadRetVals, Table::HOSES);

	REQUEST = "SELECT PK FROM HoseTests WHERE Date < 700000";
	BadRetVals = m_db.Complex(REQUEST, filter);
	printErrors("HoseTests With Bad Date Data:", BadRetVals);
	fixDate(BadRetVals, Table::HOSETESTS);
}
