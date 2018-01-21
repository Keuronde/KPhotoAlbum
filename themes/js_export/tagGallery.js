var criteria = {"currentSearch":[]};
var selectedPhotos=[];
var photosUptoDate = false;
var nbMorePhotos=20;
var nbPhotosToDisplay=nbMorePhotos;

/* Should look like this
"currentSearch":
  [
    {
      "category":categorie1,
      "values":[value1, value2],
      "boolOp":AND
    },
    {
      "categorie":categorie2,
      "values":[value1, value2],
      "boolOp":AND
    }


*/

// Function to get unique value in a array
function unique(a) {
  return a.sort().filter(function(value, index, array) {
    return (index === 0) || (value !== array[index-1]);
  });
}

function uniqueObjectInArray(myArray, property) {
  for(var i = 0; i < myArray.length; i++) {
    for(var j = i+1; j < myArray.length; j++) {
      if (myArray[i][property] === myArray[j][property]){
        myArray.splice(j,1);
      }
    }
  }
  return myArray;
}

function arrayObjectIndexOf(myArray, searchTerm, property) {
    for(var i = 0, len = myArray.length; i < len; i++) {
        if (myArray[i][property] === searchTerm) {
            return i;
        }
    }
    return -1;
}

// Function to intersect array
function intersect_photos(a, b) {
    var t;
    if (b.length > a.length) t = b, b = a, a = t; // indexOf to loop over shorter
    return a.filter(function (e) {
        return arrayObjectIndexOf(b,e.file,"file") > -1;
    });
}
function tg_reset(){
  criteria = {"currentSearch":[]};
  nbPhotosToDisplay=nbMorePhotos;
  photosUptoDate = false;
}

function tg_toggleBoolOp(myBoolOp){
  /* This function change the boolean operation that shoul be done on criteria of one category.
  Inputs:
  * myBoolOp = {"category":xxx, "boolOp":"AND" or "OR"}
  */
  
  var newBoolOp, cat;
  // Define new boolean operation
  if(myBoolOp.boolOp == "AND"){
    newBoolOp = "OR"
  }else{
    newBoolOp = "AND"
  }
  
  // Find category and apply
  for (cat=0; cat<criteria.currentSearch.length; cat++){
    if(criteria.currentSearch[cat].category == myBoolOp.category){
      criteria.currentSearch[cat].boolOp = newBoolOp;
    }
  }
  photosUptoDate = false;
  
}

function tg_getCriteriaToRenderSearchPanel(){
  rendercriteria = criteria;
  
  for(var cat=0; cat < rendercriteria.currentSearch.length; cat++){
    if(rendercriteria.currentSearch[cat].values.length > 1){
      rendercriteria.currentSearch[cat].onlyone=false;
    }else{
      rendercriteria.currentSearch[cat].onlyone=true;
    }
  }
  return rendercriteria;
}

function tg_getMorePhotos(){
	nbPhotosToDisplay= nbPhotosToDisplay + nbMorePhotos;
}

function tg_AreAllPhotosDisplayed(){
	console.log("nbPhotosToDisplay : " + nbPhotosToDisplay + ", selectedPhotos.length : " + selectedPhotos.length);
	return (nbPhotosToDisplay >= selectedPhotos.length);
}


function tg_delCriteria(myCrit){
  /* This function remove a criterion from the Criteria variable.
  Inputs:
  * myCrit = {"category":xxx, "value":xxx}
  */
  var cat;
  var val;
  var found=false;
  for (cat=0; cat<criteria.currentSearch.length; cat++){
    if(criteria.currentSearch[cat].category == myCrit.category){
      // Category found
      // If the category is found, remove the value
     	myIndex = criteria.currentSearch[cat].values.indexOf(myCrit.value);
   	  criteria.currentSearch[cat].values.splice(myIndex,1);
     	  
   	  // If there are no more values, remove the category
   	  if(criteria.currentSearch[cat].values.length == 0){
   	    criteria.currentSearch.splice(cat,1);
   	  }
   	  // No need to continue the for loop
     	break;
    }
  }

  photosUptoDate = false;
  
}

function tg_addCriteria(myCrit){
  /* This function add a criterion from the Criteria variable.
  Inputs:
  * myCrit = {"category":xxx, "value":xxx}
  */
  // Search if the category exists
  var cat;
  var categoryIsPresent=false;
  for (cat=0; cat<criteria.currentSearch.length; cat++){
    if(criteria.currentSearch[cat].category == myCrit.category){
      // If the category is found, add the value
      criteria.currentSearch[cat].values.push(myCrit.value)
      categoryIsPresent=true
    }
  }
  
  // If the category is not found, ad it with the value
  if (categoryIsPresent==false){
    criteria.currentSearch.push({
      "category":myCrit.category,
      "values":[myCrit.value],
      "boolOp":"AND"
    });
  }

  photosUptoDate = false;

}

function tg_getTags(photosDatabase, tagFamily){
  /* This function return a list of tags
  Inputs:
  * photosDatabase: database
  * tagFamily: tag family wanted */
  var nbTag = 0;
  var myTags = [];
  var i;
  for (i = 0; i < photosDatabase.Categories.length; i++) {
    if (photosDatabase.Categories[i].category == tagFamily){
      myTags[nbTag] = photosDatabase.Categories[i].value
      nbTag++;
    }
  }
  return myTags;
}

function tg_getTagFamilies(photosDatabase){
  /* this function return the list of tag family
  Inputs:
  * photosDatabase: database */
  myTagFamilies = [];
  for (i = 0; i < photosDatabase.Categories.length; i++) {
    myTagFamilies.push(photosDatabase.Categories[i].category)
  }
  myTagFamilies = unique(myTagFamilies);
  return myTagFamilies;
}

function computePhotos(photosDatabase){
		var i;
		var cat;
		var val;
	  selectedPhotos=[]

		
		
		if (criteria.currentSearch.length == 0){
		  for (i=0; i<photosDatabase.Images_data.length; i++){
		    selectedPhotos.push({"file":photosDatabase.Images_data[i].file});
		  }
		  return selectedPhotos;
		}
		var first=true;

		for (cat=0; cat<criteria.currentSearch.length; cat++){
		  // for each given categories, find corresponding photos
		  var photosForThisCategory=[];
		  var firstValue=true;
		  for(val=0; val<criteria.currentSearch[cat].values.length; val++){
		    var photosForThisValue=[];
		    
		    for(i=0; i<photosDatabase.Relations.length; i++){
		      
		      if(photosDatabase.Relations[i].category == criteria.currentSearch[cat].category &&
		         photosDatabase.Relations[i].value == criteria.currentSearch[cat].values[val]){
		        
		        photosForThisValue.push({"file":photosDatabase.Relations[i].file});
		      }
		    } // FOR Relations
		    
		    if(criteria.currentSearch[cat].boolOp == "AND"){
		      // AND logic

		      if(firstValue == true){
		        photosForThisCategory = photosForThisValue;
		        firstValue = false;
		      }else{
		        photosForThisCategory = intersect_photos(photosForThisCategory, photosForThisValue);
		      }
		    }else{
		      // OR logic
		      if(firstValue == true){
		        photosForThisCategory = photosForThisValue;
		        firstValue = false;
		      }else{
		        photosForThisCategory = uniqueObjectInArray(photosForThisCategory.concat(photosForThisValue),"file");
		      }
		    }

		  } // FOR values


		  // then get the intersection of each result
		  if (first == true){
		    selectedPhotos = photosForThisCategory;
		    first=false;
		  }else{
		    selectedPhotos = intersect_photos(selectedPhotos, photosForThisCategory);
		  }
		  
		} // FOR Category
		
		return selectedPhotos;
}



function tg_getPhotos(photosDatabase){
  /* Return the list of photos corresponding to all given criteria
  Inputs:
  * photosDatabase: database
  * criteria: array of contidions like this [{"category":"cat1","value":"val1"}, {"category":"cat2","value":"val2"}]
  */
  if(photosUptoDate == false){
	  photosUptoDate = true;
	  console.log(nbPhotosToDisplay);
	  selectedPhotos = computePhotos(photosDatabase);
	  // Reset the number of displayed photos
	  nbPhotosToDisplay=nbMorePhotos;
	  
	  
  }
  selectedPhotosForPage = selectedPhotos.slice(0,nbPhotosToDisplay);
	return selectedPhotosForPage;

}
