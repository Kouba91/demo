package cz.kouba.jezdimedoprace.controller;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import javax.validation.Valid;

import org.springframework.security.core.annotation.AuthenticationPrincipal;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.validation.BindingResult;
import org.springframework.web.bind.annotation.ModelAttribute;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.servlet.mvc.support.RedirectAttributes;

import cz.kouba.jezdimedoprace.domain.Groupe;
import cz.kouba.jezdimedoprace.domain.Ride;
import cz.kouba.jezdimedoprace.domain.User;
import cz.kouba.jezdimedoprace.domain.command.GroupeCommand;
import cz.kouba.jezdimedoprace.domain.command.RideCommand;
import cz.kouba.jezdimedoprace.domain.converter.RideToRideCommand;
import cz.kouba.jezdimedoprace.service.FormHandlerService;
import cz.kouba.jezdimedoprace.service.GroupeService;
import cz.kouba.jezdimedoprace.service.RideService;
import lombok.extern.slf4j.Slf4j;

@Slf4j
@Controller
public class MyGroupesController {

	private GroupeService groupeService;
	private FormHandlerService formHandlerService;
	private RideService rideService;
	private RideToRideCommand rideConverter;
	
	public MyGroupesController(GroupeService groupeService, FormHandlerService formHandlerService,
			RideService rideService, RideToRideCommand rideConverter) {
		this.groupeService = groupeService;
		this.formHandlerService = formHandlerService;
		this.rideService = rideService;
		this.rideConverter = rideConverter;
	}
	
	private static final byte maxNumberOfOwnedGroups = 4;

	//controller method to show all groups for user
	@RequestMapping("/myGroups")
	public String myGroups(Model model, @AuthenticationPrincipal User user) {
				
		model.addAttribute("user", user);
		//looks for groups, that include this user
		Set<Groupe> groupes = groupeService.findAllGroupsWithUser(user);
		//check if some groups were found, if not, show message, that user isnt in any group
		if (groupes.size() == 0) model.addAttribute("noGroupsFound", true);
		//add all groups (if any found) to model attribute
		model.addAttribute("groups", groupes);
		model.addAttribute("groupsCount", groupes.size());
				
		return "myGroups/myGroups";
	}
	
	//GET controller for creation of new group
	@RequestMapping(value = "/createGroup", method = RequestMethod.GET)
	public String createGroupGet(Model model, @AuthenticationPrincipal User user,
			RedirectAttributes redirectAttributes) {
		model.addAttribute("user", user);
		//check if model already contains "newGroup" attribute.
		//This is needed because of validation, so the data entered by user are preserved.
		//if not, add new plain model attribute for new group
		if (!model.containsAttribute("newGroup")) model.addAttribute("newGroup", new GroupeCommand());
		
		//check if user is owner of 4 or more groups. If so, tell him, he has reached maximum number of owned groups
		if (groupeService.countGroupsThatUserOwnsByUserId(user.getId()) >= maxNumberOfOwnedGroups) {
			redirectAttributes.addFlashAttribute("maximumNumberOfCreatedGroupsReached", true);
			return "redirect:/myGroups";
		}
			
		return "myGroups/createGroup";
	}
	
	//POST controller for creation of new group
	@RequestMapping(value = "/createGroup", method = RequestMethod.POST)
	public String createGroupPost(Model model, RedirectAttributes redirectAttributes,
			@Valid @ModelAttribute("newGroup") GroupeCommand newGroup,
			BindingResult bindingResult,
			@AuthenticationPrincipal User user) {
		
			//Binding results checks for errors in form. BindingResult bindingResult
			//has to come directly after object, that is to be validated.
			//Validation requirements are specified on in each domain object by proper valid annotation. 
			//Error messages, that are to be shown in html, are in messages properties files in section ERRORS
			if (bindingResult.hasErrors()) {
				//log all the validation errors
				bindingResult.getAllErrors().forEach(objectError -> {log.error(objectError.toString());
				});
				
				model.addAttribute("user", user);						
				return "myGroups/createGroup";
			}		
			
			//this handles all possible cases of inputs in destination and home objects
			//method formHandlerService returns model attributes, based on found errors
			redirectAttributes = formHandlerService.handleNewGroupFormMapycz(newGroup, redirectAttributes);
			
			//if there was something wrong, user will be notified of all errors on the create group page.
			//All his input will be preserved in the form
			//Check, whether redirectAttributes contains attribute of name 
			//"errorFound" if so, return errors back to create group page
			if (redirectAttributes.getFlashAttributes().containsKey("errorFound")) {
				return "redirect:/createGroup";
			}
			
			//beyond this point, everything is good to create new group
			
			try{
				//at this point, when using this method, it must be already ensured, that if data are not populated
				//the city found in database just by name will be unique, otherwise an exception will be thrown
				//This is handled by formHandlerService.handleNewGroupFormMapycz
				Groupe groupe = groupeService.createNewGroupeFromForm(newGroup, user);
				groupeService.save(groupe);
				//if there was some unexpected error while creating new group, catch exception and show default error
			} catch (Exception e) {
				log.error("Caught Exception while creating new group in GroupeService.class, method createNewGroupeFromForm");
				redirectAttributes.addFlashAttribute("defaultError", true);
				return "redirect:/createGroup";
			}
			//past this point, group was successfully created
			redirectAttributes.addFlashAttribute("newGroupCreated", true);
			
			return "redirect:/myGroups";
	}
	
	//Controller to show main group page.
	//requires group id parameter
	@RequestMapping(value = "/myGroup")
	public String myGroup(Model model, @AuthenticationPrincipal User user,
			@RequestParam(value = "id", required = true) Long id) {

			model.addAttribute("user", user);
			//find the requested group with group id and user id, just to check, if user is in that group
			Groupe groupe = groupeService.findGroupWithGroupIdAndUserId(id, user.getId());
			//if no group was found (or user is not member of that group), TODO: throw new illegal exception
			if (groupe == null) return "redirect:/";
			//at this point, fetch all members of that group
			//TODO: make all this in just one database search? 
			//check if user is in the group by "contains" user? (BUT might need to fetch the user anyway, so it is worthless)
			groupe = groupeService.findGroupWithGroupIdAndFetchUsersEagerly(id);
			//add group attribute and to model. classActiveRides is to show rides tab
			model.addAttribute("group", groupe);
			model.addAttribute("classActiveRides", true);
			//fetch rides for 8 days (0(today) - 7) 
			List<Ride> rides = rideService.findRidesForActualWeekByGroupIdAndFetchPassengersAndLocationsEagerly(id);
			//arrange locations in rides by distance from meeting
			rides = rideService.arrangeLocationsInRidesByDistanceFromMeeting(rides);
			//arrange locations in rides by order number
			rides = rideService.arrangeLocationsInRidesByOrderNumber(rides);
			//convert each Ride to RideCommand. This is needed for JSON conversion. JSON cant manage domains with hibernate relations
			List<RideCommand> ridesCommand = new ArrayList<>();
			for (Ride ride : rides) {
				ridesCommand.add(rideConverter.convert(ride));
			}
			//add converted rides to model
			model.addAttribute("ridesCommand", ridesCommand);

			return "myGroups/groupMain";		
	}	
}
