var search_tpl = `
    <p style="display:inline-block;">Recherche :</p>
    <button style:"float:right" onclick="hideSearch()" id="hideSearch">Hide current search</button>
    <div id="hidableSearch">
    <!-- BEGIN recherche -->
	{{#currentSearch}}
    <div>
    <p style="display:inline-block;">{{category}}</p>
    {{^onlyone}}
    <button type="button" name="{{category}}" boolOp="{{boolOp}}" onclick="toggleBoolOp(this)">{{boolOp}}</button>
    {{/onlyone}}
    {{#values}}
    <button type="button" name="{{category}}" value="{{.}}" onclick="delCriteria(this)">{{category}} : <span style="value">{{.}}</button>
    {{/values}}
    </div>
	{{/currentSearch}}
    <!-- END recherche -->
    <button style:"float:left" onclick="reset()">Reset</button></div>
`;
var tags_tpl = `
  <ul>
  {{#categories}}
    <li>{{name}}
      <ul class="value">
        <!-- BEGIN values -->
        {{#values}}
        <li><button type="button" name="{{name}}" value="{{.}}" onclick="addCriteria(this)" class="currentSearchButton">{{.}}</button></li>
        {{/values}}
        <!-- END values -->
      </ul>
    </li>
  <!-- END tags -->
  {{/categories}}  
  </ul>
`;
var photos_tpl = `
    <div>
    {{#selectedPhotos}}
      <!-- BEGIN photos -->
      <a href="{{file}}" data-gallery="mainGallery"><img alt="Photo" src="{{thumbFile}}"/></a>
      <!-- END photos -->
    {{/selectedPhotos}}
    {{#more}}
    <button type="button" onclick="more()">more...</button>
    {{/more}}
    </div>
`;





function addCriteria(theButton){
  tg_addCriteria({"category":theButton.getAttribute("name"),"value":theButton.getAttribute("value")});
  refresh();
}

function delCriteria(theButton){
  tg_delCriteria({"category":theButton.getAttribute("name"),"value":theButton.getAttribute("value")});
  refresh();
}
function toggleBoolOp(theButton){
  tg_toggleBoolOp({"category":theButton.getAttribute("name"),"boolOp":theButton.getAttribute("boolOp")});
  refresh();
}
function hideSearch(){
  if (document.getElementById('hidableSearch').style["display"]=="none"){
    document.getElementById('hidableSearch').style["display"]="block";
    document.getElementById('hideSearch').innerHTML = "Hide search"
  }else{
    document.getElementById('hidableSearch').style["display"]="none";
    document.getElementById('hideSearch').innerHTML = "Display search"
  }
}
function more(){
    tg_getMorePhotos();
	refresh();
}

function reset()
{
  tg_reset();
  refresh();
}

function myScroll(){
  console.log("I scroll");
  var photosHeigth = 150;
  e=document.getElementById("photos");
  if( (window.screen.availHeight + window.pageYOffset ) >
      (e.offsetTop + e.offsetHeight - photosHeigth)){
    // If the bottom of the window is less than one photo row from the bottom of the page
    // Display more pictures
    more();
  }

}

// Construct categories and their value
function computeCategory()
{
  var CategoryAndValue={"categories":[]};
  var Category= tg_getTagFamilies(photosDatabase);
  var i,cat,k,val;
  for (i=0; i<Category.length; i++){
    familyTags_ref = tg_getTags(photosDatabase, Category[i])
    familyTags=[];
    for (k=0; k<familyTags_ref.length; k++){
      // do not display the value already selected
      var toAdd=true;
      for (cat=0; cat<criteria.currentSearch.length; cat++){
        // TODO : loop on the criteria.currentSearch[j].value[0]
        // TODO : make a function tg_isInCriteria({category, value})
        for (val=0; val<criteria.currentSearch[cat].values.length; val++){
          if((Category[i] == criteria.currentSearch[cat].category) && (familyTags_ref[k] == criteria.currentSearch[cat].values[val])){
            toAdd=false;
          }
        }
      }
      if(toAdd == true){      
       familyTags.push(familyTags_ref[k]);
      }
    }
    if(familyTags.length > 0){
      CategoryAndValue.categories.push({"name":Category[i],"values":familyTags})
    }
  }
  return CategoryAndValue;
}

function refresh(){
  var CategoryAndValue = computeCategory();
  var selectedPhotos = tg_getPhotos(photosDatabase);

  
  
  if(!tg_AreAllPhotosDisplayed()){
    if(  window.onscroll === null){
      	window.onscroll = function() {myScroll()};
    }
  }else{
    window.onscroll = null;
  }
  // Current search
  //Grab the inline template
  // TODO : make a function tg_criteriaIsEmpty
  if(criteria.currentSearch.length > 0){
    //Parse it (optional, only necessary if template is to be used again)
    Mustache.parse(search_tpl);
    //Render the data into the template
    var rendered = Mustache.render(search_tpl, tg_getCriteriaToRenderSearchPanel());
    //Overwrite the contents of #target with the rendered HTML
    document.getElementById('search').innerHTML = rendered;
  }else{
    document.getElementById('search').innerHTML = "";
  }
  

  //Parse it (optional, only necessary if template is to be used again)
  Mustache.parse(tags_tpl);
  //Render the data into the template
  var rendered = Mustache.render(tags_tpl, CategoryAndValue);
  //Overwrite the contents of #target with the rendered HTML
  document.getElementById('tags').innerHTML = rendered;

  // PHOTOS
  //Parse it (optional, only necessary if template is to be used again)
  Mustache.parse(photos_tpl);
  //Render the data into the template
  var rendered = Mustache.render(photos_tpl, {"selectedPhotos":selectedPhotos, "more":!tg_AreAllPhotosDisplayed()});
  //Overwrite the contents of #target with the rendered HTML
  document.getElementById('photos').innerHTML = rendered;

  // PhotoViewer:
  lfg_init();

  
}





