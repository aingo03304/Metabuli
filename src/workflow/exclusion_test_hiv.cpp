//
// Created by 김재범 on 2021/7/12.
//

//#include "Classifier.h"
#include "Parameters.h"
#include "LocalParameters.h"
#include "NcbiTaxonomy.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

using namespace std;
struct Counts13{
    int classificationCnt;
    int correct;
    int highRank;

    //number of targets at each rank
    int subspeciesTargetNumber;
    int speciesTargetNumber;
    int genusTargetNumber;
    int familyTargetNumber;
    int orderTargetNumber;
    int classTargetNumber;
    int phylumTargetNumber;
    int superkingdomTargetNumber;

    //number of classification at each rank
    int subspeciesCnt_try;
    int speciesCnt_try;
    int genusCnt_try;
    int familyCnt_try;
    int orderCnt_try;
    int classCnt_try;
    int phylumCnt_try;
    int superkingdomCnt_try;


    //number of correct classifications at each rank
    int subspeciesCnt_correct;
    int speciesCnt_correct;
    int genusCnt_correct;
    int familyCnt_correct;
    int orderCnt_correct;
    int classCnt_correct;
    int phylumCnt_correct;
    int superkingdomCnt_correct;
};

void compareTaxon112(TaxID shot, TaxID target, NcbiTaxonomy & ncbiTaxonomy, Counts13 & counts);



int exclusiontest_hiv(int argc, const char **argv, const Command &command){

    LocalParameters &par = LocalParameters::getLocalInstance();
    par.parseParameters(argc, argv, command, false, Parameters::PARSE_ALLOW_EMPTY, 0);

    const string readClassificationFileName = par.filenames[0];
    const string mappingFile = par.filenames[1];

    string names = "/data3/jaebeom/ncbi_dmp/names.dmp";
    string nodes = "/data3/jaebeom/ncbi_dmp/nodes.dmp";
    string merged = "/data3/jaebeom/ncbi_dmp/merged.dmp";
    NcbiTaxonomy ncbiTaxonomy(names, nodes, merged);

    unordered_map<TaxID, unsigned int> taxCnt;
    for(int i = 0 ; i < ncbiTaxonomy.taxonNodes.size() ; i++) {
        if(ncbiTaxonomy.taxonNode(ncbiTaxonomy.taxonNodes[i].taxId)->rank == "subspecies") {
            taxCnt[ncbiTaxonomy.taxonNodes[i].taxId] = 1;
        }
    }

    unordered_map<TaxID, TaxonCounts> cladeCnt = ncbiTaxonomy.getCladeCounts(taxCnt);
    for(auto it = cladeCnt.begin(); it != cladeCnt.end(); it ++){
        if(ncbiTaxonomy.taxonNode(it->first)->rank == "genus") {
            //cout << it->second.cladeCount << " " << it->second.taxCount << endl;
        }
    }


    Counts13 counts = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // 1) Load mapping file
    cout<<"Load mapping from accession ID to taxonomy ID"<<endl;
    unordered_map<string, int> acc2taxid;
    string eachLine;
    string eachItem;
    ifstream map;
    map.open(mappingFile);
    vector<string> items;
    if(map.is_open()){
        while(getline(map,eachLine,'\n')){
            istringstream ss(eachLine);
            while (getline(ss, eachItem, '\t')){
                items.push_back(eachItem);
            }
            acc2taxid[items[0]] = stoi(items[1]);
            items.clear();
        }
    } else{
        cout<<"Cannot open file for mapping from accession to tax ID"<<endl;
    }
    map.close();
    cout<<"Here"<<endl;

    // Load read classification
    vector<int> rightAnswers;
    vector<int> classifications;
    string classificationString;
    vector<string> fields;
    string field;
    int taxID;
    int rightAnswer;
    int rightAnswer_sp;
    unsigned int cladeCnt_sp;
    string accession;
    smatch assacc;
    ifstream readClassification;
    readClassification.open(readClassificationFileName);
    while(getline(readClassification, classificationString, '\n')){
        istringstream lineStream(classificationString);
        fields.clear();
        // 3rd field -> classification
        while(getline(lineStream, field, '\t')){
            fields.push_back(field);
        }
        taxID = stoi(fields[2]);
        classifications.push_back(taxID);

        // 2nd field -> accession
        accession = fields[1];

        // accession to right answer
        rightAnswer = acc2taxid[accession];
        rightAnswer_sp = ncbiTaxonomy.getTaxIdAtRank(rightAnswer, "species");
        cladeCnt_sp = cladeCnt[rightAnswer_sp].cladeCount;

        //get the lowest ancestor that have different branch
        const TaxonNode * ancestor = ncbiTaxonomy.taxonNode(ncbiTaxonomy.getTaxIdAtRank(rightAnswer_sp, "species"));
        while(cladeCnt_sp == cladeCnt[ancestor->taxId].cladeCount){
            ancestor = ncbiTaxonomy.taxonNode(ancestor->parentTaxId);
            if(ancestor->rank == "superkingdom"){
                break;
            }
        }
        rightAnswers.push_back(11676);

        if(ancestor->rank == "superkingdom"){
            counts.superkingdomTargetNumber ++;
        } else if(ancestor->rank == "phylum"){
            counts.phylumTargetNumber ++;
        } else if(ancestor->rank == "order"){
            counts.orderTargetNumber ++;
        } else if(ancestor->rank == "class"){
            counts.classTargetNumber ++;
        } else if(ancestor->rank == "family"){
            counts.familyTargetNumber ++;
        } else if(ancestor->rank == "genus"){
            counts.genusTargetNumber ++;
        }
    }
    cout<<"hi"<<endl;
    cout << "num of classification: " << classifications.size() << endl;

//    ///Load query file -> name
//    regex regex1("(GC[AF]_[0-9]*\\.[0-9]*)");
//    smatch assacc;
//    string queryName;
//    ifstream query;
//    query.open(queryFileName);
//    string queryLine;
//    vector<string> queryNameList;
//    while(getline(query,queryLine,'\n')){
//        if(queryLine[0] == '>'){
//            regex_search(queryLine, assacc, regex1);
//            queryNameList.push_back(assacc[0]);
//        }else{
//            continue;
//        }
//    }



//    ///right answer list
//    vector<int> rightAnswers;
//    int taxid;
//    int taxid_sp;
//    unsigned int cladeCnt_sp;
//    for(size_t i = 0; i < queryNameList.size(); i++){
//        if (assacc2taxid.count(queryNameList[i])) {
//            taxid = assacc2taxid[queryNameList[i]];
//            taxid_sp = ncbiTaxonomy.getTaxIdAtRank(taxid, "species");
//            cladeCnt_sp = cladeCnt[taxid_sp].cladeCount;
//            const TaxonNode * ancestor = ncbiTaxonomy.taxonNode(ncbiTaxonomy.getTaxIdAtRank(taxid_sp, "genus"));
//            while(cladeCnt_sp == cladeCnt[ancestor->taxId].cladeCount){
//                ancestor = ncbiTaxonomy.taxonNode(ancestor->parentTaxId);
//                if(ancestor->rank == "superkingdom"){
//                    break;
//                }
//            }
//            rightAnswers.push_back(ancestor->taxId);
//            if(ancestor->rank == "superkingdom"){
//                counts.superkingdomTargetNumber ++;
//            } else if(ancestor->rank == "phylum"){
//                counts.phylumTargetNumber ++;
//            } else if(ancestor->rank == "order"){
//                counts.orderTargetNumber ++;
//            } else if(ancestor->rank == "class"){
//                counts.classTargetNumber ++;
//            } else if(ancestor->rank == "family"){
//                counts.familyTargetNumber ++;
//            } else if(ancestor->rank == "genus"){
//                counts.genusTargetNumber ++;
//            }
//        } else{
//            cout << classifications[i] << " is not in the mapping file" << endl;
//            rightAnswers.push_back(-1);
//            continue;
//        }
//    }




    ///score the classification
    for(size_t i = 0; i < classifications.size(); i++){
        cout<<i<<" ";
        compareTaxon112(classifications[i], rightAnswers[i], ncbiTaxonomy, counts);
    }

    cout << "Num of queries: " << classifications.size() << endl;
    cout<<"Num of classifications: "<< counts.classificationCnt << endl;
    cout<<"Num of correct classifications: "<<counts.correct<<endl;
    cout<<"Num of correct but too broad classifications: "<<counts.highRank<<endl;
    cout << "classified/total = " << float(counts.classificationCnt)/float(classifications.size()) << endl;
    cout << "correct   /total = " << float(counts.correct) / float(classifications.size()) << endl;
    cout<<"correct   /classifications = "<<float(counts.correct) / float(counts.classificationCnt) <<endl;
    cout<<"high rank /classifications = "<<float(counts.highRank) / float(counts.classificationCnt) <<endl << endl;

    cout<<"Number of targets at each rank / correct classification / tries"<<endl;
    cout<<"Superkingdom: " << counts.superkingdomTargetNumber << " / " << counts.superkingdomCnt_correct << " / "<<counts.superkingdomCnt_try<<endl;
    cout<<"Phylum      : " << counts.phylumTargetNumber << " / " << counts.phylumCnt_correct << " / "<<counts.phylumCnt_try<<endl;
    cout<<"Class       : " << counts.classTargetNumber << " / " << counts.classCnt_correct << " / "<<counts.classCnt_try<<endl;
    cout<<"Order       : " << counts.orderTargetNumber<<" / "<<counts.orderCnt_correct<<" / "<<counts.orderCnt_try<<endl;
    cout<<"Family      : " << counts.familyTargetNumber << " / " << counts.familyCnt_correct << " / "<<counts.familyCnt_try<<endl;
    cout<<"Genus       : " << counts.genusTargetNumber<<" / "<<counts.genusCnt_correct<<" / "<<counts.genusCnt_try<<endl;
    cout<<"Species     : " << counts.speciesTargetNumber<<" / "<<counts.speciesCnt_correct<<" / "<<counts.speciesCnt_try<<endl;
    cout<<"Subspecies  : " << counts.subspeciesTargetNumber<<" / "<<counts.subspeciesCnt_correct<<" / "<<counts.subspeciesCnt_try<<endl;
    return 0;
}

void compareTaxon112(TaxID shot, TaxID target, NcbiTaxonomy & ncbiTaxonomy, Counts13 & counts) { ///target: subspecies or species
    const TaxonNode * shotNode = ncbiTaxonomy.taxonNode(shot);
    const TaxonNode * targetNode = ncbiTaxonomy.taxonNode(target);
    string shotRank = shotNode->rank;
    string targetRank = targetNode->rank;
    cout<<shot<<" "<<target<<" "<<shotRank<<" "<<targetRank<<" ";

    if(shot == 0){
        cout<<"X"<<endl;
        return;
    } else{
        counts.classificationCnt++;
    }

    bool isCorrect = false;
    if(shot == target){
        counts.correct ++;
        isCorrect = true;
        cout<<"O"<<endl;
    } else if(NcbiTaxonomy::findRankIndex(shotRank) <= NcbiTaxonomy::findRankIndex(targetRank)){ //classified into wrong taxon or too specifically
        cout<<"X"<<endl;
    } else { // classified at higher rank (too safe classification)
        if(shotRank == "superkingdom"){
            cout<<"X"<<endl;
        } else if(shot == ncbiTaxonomy.getTaxIdAtRank(target, shotRank)){ //on right branch
            counts.highRank ++;
            cout<<"U"<<endl;
        } else{ //on wrong branch
            cout<<"X"<<endl;
        }
    }

    //count the number of classification at each rank
    if(shotRank == "subspecies") {
        counts.subspeciesCnt_try++;
    } else if(shotRank == "species") {
        counts.speciesCnt_try ++;
    } else if(shotRank == "genus"){
        counts.genusCnt_try ++;
    } else if(shotRank == "family"){
        counts.familyCnt_try++;
    } else if(shotRank == "order") {
        counts.orderCnt_try++;
    } else if(shotRank == "class") {
        counts.classCnt_try++;
    } else if(shotRank == "phylum") {
        counts.phylumCnt_try++;
    } else if(shotRank == "superkingdom"){
        counts.superkingdomCnt_try++;
    }

    if(!isCorrect) return;

    //count the number of correct classification at each rank
    if(shotRank == "subspecies"){
        counts.subspeciesCnt_correct ++;
    } else if(shotRank == "species") {
        counts.speciesCnt_correct ++;
    } else if(shotRank == "genus"){
        counts.genusCnt_correct ++;
    } else if(shotRank == "family"){
        counts.familyCnt_correct++;
    } else if(shotRank == "order") {
        counts.orderCnt_correct++;
    } else if(shotRank == "class") {
        counts.classCnt_correct++;
    } else if(shotRank == "phylum") {
        counts.phylumCnt_correct++;
    } else if(shotRank == "superkingdom"){
        counts.superkingdomCnt_correct++;
    }
}

